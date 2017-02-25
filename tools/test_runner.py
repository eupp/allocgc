import os
import re
import sys
import json
import math
import getopt
import logging
import tempfile
import itertools
import subprocess
import collections


STATS_FULL_TIME_MEAN    = "full time mean"
STATS_FULL_TIME_STD     = "full time std"
STATS_GC_TIME_MEAN      = "gc time mean"
STATS_GC_TIME_STD       = "gc time std"
STATS_STW_TIME_MEAN     = "stw time mean"
STATS_STW_TIME_STD      = "stw time std"
STATS_STW_TIME_MAX      = "stw time max"
STATS_STW_TIME_MIN      = "stw time min"
STATS_GC_COUNT          = "gc count"

STATS_PAUSES = "pauses"
STATS_HEAP_SIZE = "heap size"
STATS_HEAP_LIVE = "heap live"
STATS_HEAP_OCCUPIED = "heap occupied"
STATS_HEAP_SWEPT = "heap swept"
STATS_HEAP_COPIED = "heap copied"

def mean(xs):
    s = 0.0
    for x in xs:
        s += x
    res = s / len(xs)
    return s / len(xs)


def std(xs):
    return math.sqrt(mean([x**2 for x in xs]) - mean(xs)**2)


def stat_mean(arr, default=float('NaN'), ndigits=0):
    return round(mean(arr), ndigits) if len(arr) > 0 else default


def stat_std(arr, default=float('NaN'), ndigits=0):
    return round(std(arr), ndigits) if len(arr) > 0 else default


def stat_max(arr, default=float('NaN'), ndigits=0):
    return round(max(arr), ndigits) if len(arr) > 0 else default


def stat_min(arr, default=float('NaN'), ndigits=0):
    return round(min(arr), ndigits) if len(arr) > 0 else default


def call_with_cwd(args, cwd):
    logging.info("{}: {}".format(cwd, " ".join(args)))
    proc = subprocess.Popen(args, cwd=cwd)
    proc.wait()
    assert proc.returncode == 0


def call_output(args, cwd=None, timeout=None):
    logging.info("{}: {}".format(cwd, " ".join(args)))
    proc = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        out, err = proc.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        raise
    return proc.returncode, out.decode("utf-8")


class CMakeBuilder:

    def __init__(self, src_dir, *args):
        self._tmpdir = tempfile.TemporaryDirectory()
        build_cmd  = ["cmake", "-DCMAKE_BUILD_TYPE=Release", src_dir]
        build_cmd += self._parse_cmake_options(args)
        call_with_cwd(build_cmd, self._tmpdir.name)
        self._make_builder = MakeBuilder(self._tmpdir.name)

    def build(self, target):
        return self._make_builder.build(target)

    def build_dir(self):
        return self._tmpdir.name

    @staticmethod
    def _parse_cmake_options(flags):
        res = []
        for flag in flags:
            if isinstance(flag, str):
                res.append("-D{}=ON".format(flag))
            elif isinstance(flag, dict):
                for k, v in flag.items():
                    if v is None:
                        res.append("-D{}=ON".format(k))
                    else:
                        res.append("-D{}={}".format(k, v))
        return res


class MakeBuilder:

    def __init__(self, build_dir):
        self._build_dir = build_dir

    def build(self, target):
        call_with_cwd(["make", target], self._build_dir)
        return MakeBuilder.Build(self._build_dir)

    def build_dir(self):
        return self._build_dir

    class Build:

        def __init__(self, dirname):
            self._dirname = dirname

        def run(self, runnable, args):
            runnable = os.path.join(self.dirname(), runnable)
            call_args = [runnable] + args
            return call_output(call_args, cwd=self.dirname(), timeout=30)

        def dirname(self):
            return self._dirname


class Scanner:

    def __init__(self, token_spec):
        self._ts = {}
        for token, spec in token_spec.items():
            self._ts[token] = {"cmd": spec["cmd"], "re": re.compile(spec["re"])}

    def scan(self, text):
        for line in text.splitlines():
            for token, spec in self._ts.items():
                match = spec["re"].match(line)
                if match is not None:
                    action = spec["cmd"]
                    action(match)


class Parser:

    def __init__(self):

        def parse_full_time(match):
            self._context["full_time"] += [int(match.group("full_time"))]

        def parse_gc_time(match):
            self._context["gc_time"] += [int(match.group("gc_time"))]

        def parse_stw_time(match):
            self._context["stw_time"] += [int(match.group("stw_time"))]

        def parse_gc_count(match):
            self._context["gc_count"] += [int(match.group("gc_count"))]

        def parse_size(match, key):
            sz = int(match.group(key))
            self._context[key].append(sz)

        def parse_heap_size(match):
            parse_size(match, "heap_size")

        def parse_occupied(match):
            parse_size(match, "occupied")

        def parse_live(match):
            parse_size(match, "live")

        def parse_swept(match):
            parse_size(match, "swept")

        def parse_copied(match):
            parse_size(match, "copied")

        def parse_pause_time(match):
            self._context["pause_time"] += [int(match.group("pause_time"))]

        token_spec = {
            "FULL_TIME" : {"cmd": parse_full_time, "re": "Completed in (?P<full_time>\d*) ms"},
            "GC_TIME"   : {"cmd": parse_gc_time, "re": "Time spent in gc (?P<gc_time>\d*) ms"},
            "STW_TIME"  : {"cmd": parse_stw_time, "re": "Average pause time (?P<stw_time>\d*) us"},
            "GC_COUNT"  : {"cmd": parse_gc_count, "re": "Completed (?P<gc_count>\d*) collections"},
            "HEAP_SIZE" : {"cmd": parse_heap_size, "re": "heap size: \s*(?P<heap_size>\d*) b "},
            "OCCUPIED"  : {"cmd": parse_occupied, "re": "occupied: \s*(?P<occupied>\d*) b "},
            "LIVE"      : {"cmd": parse_live, "re": "live: \s*(?P<live>\d*) b "},
            "SWEPT"     : {"cmd": parse_swept, "re": "swept: \s*(?P<swept>\d*) b "},
            "COPIED"    : {"cmd": parse_copied, "re": "copied: \s*(?P<copied>\d*) b "},
            "PAUSE_TIME": {"cmd": parse_pause_time, "re": "pause time: \s*(?P<pause_time>\d*) us"}

        }

        self._scanner = Scanner(token_spec)
        self._context = collections.defaultdict(list)

    def parse(self, test_output):
        self._scanner.scan(test_output)

    def result(self):
        return {
            STATS_FULL_TIME_MEAN  : stat_mean(self._context["full_time"], ndigits=0),
            STATS_FULL_TIME_STD   : stat_std(self._context["full_time"], ndigits=3),
            STATS_GC_TIME_MEAN    : stat_mean(self._context["gc_time"], ndigits=0),
            STATS_GC_TIME_STD     : stat_std(self._context["gc_time"], ndigits=3),
            STATS_STW_TIME_MEAN   : self._us_to_ms(stat_mean(self._context["stw_time"]), ndigits=3),
            STATS_STW_TIME_STD    : self._us_to_ms(stat_std(self._context["stw_time"]), ndigits=3),
            STATS_STW_TIME_MAX    : self._us_to_ms(stat_max(self._context["stw_time"]), ndigits=3),
            STATS_STW_TIME_MIN    : self._us_to_ms(stat_min(self._context["stw_time"]), ndigits=3),
            STATS_GC_COUNT        : stat_mean(self._context["gc_count"], default=0, ndigits=0),

            STATS_HEAP_SIZE     : self._context["heap_size"],
            STATS_HEAP_OCCUPIED : self._context["occupied"],
            STATS_HEAP_LIVE     : self._context["live"],
            STATS_HEAP_SWEPT    : self._context["swept"],
            STATS_HEAP_COPIED   : self._context["copied"],
            STATS_PAUSES        : self._context["pause_time"]
        }

    @staticmethod
    def _us_to_ms(us, ndigits=3):
        return round(float(us) / 1000, ndigits)


def create_parser(target):
    if target in ("boehm", "multisize_boehm", "parallel_merge_sort", "pyboehm", "csboehm", "cord"):
        return Parser()


class Report:

    def __init__(self, saved=None):
        self._cols = ["name",
                      STATS_GC_COUNT,
                      STATS_FULL_TIME_MEAN, STATS_FULL_TIME_STD,
                      STATS_GC_TIME_MEAN, STATS_GC_TIME_STD,
                      STATS_STW_TIME_MEAN, STATS_STW_TIME_STD, STATS_STW_TIME_MIN, STATS_STW_TIME_MAX,
                      STATS_HEAP_SIZE, STATS_HEAP_OCCUPIED, STATS_HEAP_LIVE, STATS_HEAP_SWEPT, STATS_HEAP_COPIED,
                      STATS_PAUSES]
        self._targets = []
        self._table = collections.defaultdict(list)

        if saved is not None:
            for target, data in saved:
                self._targets.append(target)
                self._table[target] = data

    def add_stats(self, target, row):
        if target not in self._targets:
            self._targets.append(target)
        self._table[target].append(self._match_columns(row, self._cols))

    def cols(self):
        return self._cols

    def items(self):
        return [(target, self._table[target]) for target in self._targets]

    def targets_num(self):
        return len(self._targets)

    @staticmethod
    def _match_columns(row, cols):
        res = []
        for col in cols:
            res.append(row.get(col))
        return res


class JSONPrinter:

    def __init__(self, outfn):
        self._outfn = outfn

    def print_report(self, report):
        with open(self._outfn, "w") as outfile:
            json.dump(report.items(), outfile)

    def outfn(self):
        return self._outfn


class TexTablePrinter:

    def print_report(self, report):
        import tabulate
        for target, data in report.items():
            outfn = target + ".tex"
            with open(outfn, "w") as outfile:
                outfile.write(tabulate.tabulate(data, report.cols(), tablefmt="latex"))


class TimeBarPlotPrinter:

    def __init__(self, outfn):
        self._outfn = outfn

    def print_report(self, report):
        import matplotlib.pyplot as plt
        n = report.targets_num()

        for target, data in report.items():
            ind   = range(0, len(data))
            width = 0.5
            names  = ["\n".join(row[0].split(' ')) for row in data]
            means  = [row[2] for row in data]
            stds   = [row[3] for row in data]

            rects = plt.bar(ind, means, width, yerr=stds, color='r')

            # add some text for labels, title and axes ticks
            plt.title(target)
            plt.ylabel("Time (ms)")
            # plt.xticks([x + width/2 for x in ind])
            plt.xticks(ind)
            plt.gca().set_xticklabels(names)
            # plt.xlim(-1, len(data)+1)
            # ax.set_yticks(range(0, 5500, 500))
            # ax.set_ylim([0, 5500])

            outfn = self._outfn + '-' + target + '.eps'

            plt.savefig(outfn)


class PauseTimePlotPrinter:

    def __init__(self, outfn):
        self._outfn = outfn

    def print_report(self, report):
        import matplotlib.pyplot as plt
        n = report.targets_num()

        for target, data in report.items():

            ind   = range(0, len(data))
            width = 0.5
            names  = ["\n".join(row[0].split(' ')) for row in data]
            means  = [row[6] for row in data]
            stds    = [row[7] for row in data]

            rects = plt.bar(ind, means, width, yerr=stds, color='b', ecolor='black')

            # create stacked errorbars:
            # ax.errorbar(range(0, m), means, std, fmt='ok', lw=1)
            # ax.errorbar(range(0, m), means, [mins, maxes],
            #              fmt='.k', ecolor='gray', lw=1)
            plt.xlim(-1, len(data)+1)

            plt.title(target)
            plt.ylabel("Pause Time (ms)")
            plt.xticks([x + width/2 for x in ind])
            plt.gca().set_xticklabels(names)
            # ax.set_ylim([min(means) - 0.2, max(means) + 0.5])

            outfn = self._outfn + '-' + target + '.eps'

            plt.tight_layout()
            plt.savefig(outfn)

class GCTimePlotPrinter:

    def __init__(self, outfn, rownum, figsize):
        self._outfn = outfn
        self._rownum = int(rownum)
        self._figsize = figsize

    def print_report(self, report):
        import matplotlib.pyplot as plt
        n = report.targets_num()
        rownum = self._rownum
        colnum = math.ceil(float(n) / rownum)

        fig, axs = plt.subplots(rownum, colnum, figsize=self._figsize)

        i, j = 0, 0
        for target, data in report.items():
            ax = axs[i, j]

            j += 1
            if j == colnum:
                i += 1
                j = 0

            ind   = range(0, len(data))
            width = 0.5
            names  = ["\n".join(row[0].split(' ')) for row in data]
            means  = [row[4] for row in data]
            stds   = [row[5] for row in data]

            rects = ax.bar(ind, means, width, color='b')

            # add some text for labels, title and axes ticks
            ax.set_title(target)
            ax.set_ylabel("GC Time (ms)")
            ax.set_xticks(ind)
            ax.set_xticklabels(names)
            # ax.set_yticks(range(0, 5500, 500))
            # ax.set_ylim([0, 5500])

        fig.tight_layout()
        fig.savefig(self._outfn)

class HeapPlotPrinter:

    def __init__(self, outfn):
        self._outfn = outfn

    def print_report(self, report):
        import matplotlib.pyplot as plt
        n = report.targets_num()

        for target, data in report.items():
            plt.clf()

            m = len(data)
            fig, axs = plt.subplots(m)
            for i, row in zip(range(0, m), data):
                name       = row[0]
                before_gcs = row[10]
                after_gcs  = row[11]

                if name == "gc_ptr concurrent-mark":
                    before_gcs = list(filter(lambda x: x != 0, before_gcs))
                    after_gcs  = list(filter(lambda x: x != 0, after_gcs))

                ax = axs[i]
                x  = range(0, len(before_gcs))
                ax.fill_between(x, before_gcs, 0, color='gray')
                ax.fill_between(x, after_gcs, 0, color='black')


                ax.set_title(name)
                ax.set_ylabel('Memory (bytes)')

            outfn = self._outfn + '-' + target + '.eps'

            plt.title(target)
            plt.tight_layout()
            plt.savefig(outfn)




def create_printer(printer_name, *args, **kwargs):
    if printer_name == "json":
        return JSONPrinter(*args, **kwargs)
    if printer_name == "tex-table":
        return TexTablePrinter(*args, **kwargs)
    if printer_name == "time-bar-plot":
        return TimeBarPlotPrinter(*args, **kwargs)
    if printer_name == "pause-time-plot":
        return PauseTimePlotPrinter(*args, **kwargs)
    if printer_name == "heap-plot":
        return HeapPlotPrinter(*args, **kwargs)
    if printer_name == "gc-time-plot":
        return GCTimePlotPrinter(*args, **kwargs)


class RunChecker:

    def __init__(self, maxn=None, assert_on_fail=False):
        self._runs = collections.defaultdict(lambda: collections.defaultdict(int))
        self._maxn = maxn
        self._assert_on_fail = assert_on_fail

    def check_run(self, target, run_name, rc):
        name = self._full_name(target, run_name)
        self._runs[name]["run count"] += 1
        # if self._maxn is not None and self._runs[name]["run count"] > self._maxn:
        #     assert False, "Too many runs"
        if rc != 0:
            if self._assert_on_fail:
                assert False, "Run failed with error code %d" % rc
            self._runs[name]["failed count"] += 1
        return rc == 0

    def interrupted_run(self, target, run_name):
        name = self._full_name(target, run_name)
        self._runs[name]["run count"] += 1
        # if self._maxn is not None and self._runs[name]["run count"] > self._maxn:
        #     assert False, "Too many runs"
        self._runs[name]["interrupted count"] += 1

    def results(self):
        str = ""
        for k, v in self._runs.items():
            str += k + ":\n"
            str += "\n".join("  %s = %s" % (kk, vv) for (kk, vv) in v.items())
            str += "\n"
        return str

    @staticmethod
    def _full_name(target, run_name):
        return target + " " + run_name


class Target:

    def __init__(self, name, alias, runnable, suites):
        self._name      = name
        self._alias     = alias
        self._suites    = suites
        self._runnable  = runnable

    def run(self, params_space, report, checker):
        for suite in self._suites:
            build = suite["builder"].build(self._name)
            for params in params_space:
                params = Target.Param.to_program_args(params)
                run_name = " ".join([suite["name"]] + params)
                args = params + suite["args"]
                parser = create_parser(self._name)
                n = 0
                while n < nruns:
                    try:
                        rc, output = build.run(self._runnable, args)
                    except subprocess.TimeoutExpired:
                        logging.info("Interrupted (timeout expired)!")
                        checker.interrupted_run(self._alias, run_name)
                        continue
                    logging.info("Return code: {}".format(rc))
                    logging.debug("Output: \n {}".format(output))
                    if checker.check_run(self._alias, run_name, rc):
                        logging.info("Parse output")
                        parser.parse(output)
                        n += 1

                parsed = parser.result()
                logging.debug("Parsed: \n {}".format(json.dumps(parsed)))

                logging.info("Add parser output to reporter")
                parsed["name"]   = suite["name"]
                parsed["params"] = params

                report.add_stats(self._alias, parsed)

    @staticmethod
    def parse_params(params):
        params_space = []
        for param in params:
            space = []
            if isinstance(param, str):
                space.append(Target.Param(param))
            elif isinstance(param, dict):
                assert len(param) == 1
                for k, v in param.items():
                    if isinstance(v, list):
                        for arg in v:
                            space.append(Target.Param(k, arg))
                    else:
                        space.append(Target.Param(k, v))
            params_space.append(space)
        return list(itertools.product(*params_space))

    class Param:

        def __init__(self, name, arg = None):
            self._name = name
            self._arg  = arg

        @staticmethod
        def to_program_args(params):
            res = []
            for param in params:
                res.append("--{}".format(param._name))
                if param._arg is not None:
                    res.append(" {}".format(str(param._arg)))
            return res

PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")


if __name__ == '__main__':

    print(sys.version)
    print(os.getcwd())

    opts, args = getopt.getopt(sys.argv[1:], "", ["cfg=", "cmake-dir=", "use-saved-data"])
    cmake_dir = None
    use_saved = False
    for opt, arg in opts:
        if opt == "--cfg":
            config = arg
        elif opt == "--cmake-dir":
            cmake_dir = arg
        elif opt == "--use-saved-data":
            use_saved = True

    logger = logging.getLogger()
    logger.setLevel(logging.INFO)

    with open(config) as fd:
        cfg = json.load(fd)


    printers = {}
    for printer in cfg.get("printers", []):
        name = printer["name"]
        printers[name] = create_printer(name, **printer.get("params", {}))

    if not use_saved:
        nruns       = cfg.get("nruns")
        failquick   = cfg.get("failquick", False)

        builders = {}
        for builder in cfg["builders"]:
            name = builder["name"]
            type = builder["type"]

            if type == "cmake":
                if cmake_dir is not None:
                    builders[name] = MakeBuilder(cmake_dir)
                else:
                    builders[name] = CMakeBuilder(PROJECT_DIR, *builder["options"])

        suites = {}
        for suite, i in zip(cfg["suites"], range(0, len(cfg["suites"]))):
            name = suite["name"]

            suites[name] = {
                "name": name,
                "builder": builders[suite["builder"]],
                "args": suite.get("args", []),
                "ord": i
            }

        suites = dict(sorted(suites.items(), key=lambda x: x[1]["ord"]))

        report = Report()
        checker = RunChecker(maxn=3*nruns, assert_on_fail=failquick)
        for target in cfg["targets"]:
            trgt_name       = target["name"]
            trgt_alias      = target.get("alias", trgt_name)
            trgt_suites     = [suites[name] for name in target["suites"]]
            trgt_params     = Target.parse_params(target.get("params", []))
            trgt = Target(trgt_name, trgt_alias, target["runnable"], trgt_suites)

            trgt.run(trgt_params, report, checker)
            print(checker.results())
    else:
        saved_fn = printers["json"].outfn()
        with open(saved_fn) as saved:
            report = Report(json.load(saved))

    logging.info("Produce reports")
    for printer in printers.values():
        printer.print_report(report)


