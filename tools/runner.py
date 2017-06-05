import os
import sys
import json
import getopt
import logging
import tempfile
import itertools
import subprocess
import collections

import parsers
import printers

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


def create_parser(name, *args, **kwargs):
    if name == "gc-time":
        return parsers.GCTimeParser(*args, **kwargs)


def create_printer(name, *args, **kwargs):
    if name == "json":
        return printers.JSONPrinter(*args, **kwargs)
    # if name == "tex-table":
    #     return TexTablePrinter(*args, **kwargs)
    # if name == "time-bar-plot":
    #     return TimeBarPlotPrinter(*args, **kwargs)
    # if name == "pause-time-plot":
    #     return PauseTimePlotPrinter(*args, **kwargs)
    # if name == "heap-plot":
    #     return HeapPlotPrinter(*args, **kwargs)
    # if name == "gc-time-plot":
    #     return GCTimePlotPrinter(*args, **kwargs)
    # if name == "total-time-text":
    #     return TextTotalTimePrinter(*args, **kwargs)
    # if name == "pause-time-text":
    #     return TextPauseTimePrinter(*args, **kwargs)


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


class Report:

    def __init__(self):
        self.suites  = collections.defaultdict(dict)
        self.targets = collections.defaultdict(dict)

    def add_stats(self, target, suite, data):
        self.suites[suite][target]  = data
        self.targets[target][suite] = data


class Target:

    def __init__(self, name, alias, runnable, suites):
        self._name      = name
        self._alias     = alias
        self._suites    = suites
        self._runnable  = runnable

    def run(self, params_space, report, parsers, checker):
        for suite in self._suites:
            build = suite["builder"].build(self._name)
            for params in params_space:
                params = Target.Param.to_program_args(params)
                run_name = " ".join([suite["name"]] + params)
                args = params + suite["args"]
                n = 0
                for parser in parsers:
                    parser.reset()

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
                        for parser in parsers:
                            parser.parse(output)
                        n += 1

                stats = {}
                for parser in parsers:
                    parsed = parser.result()
                    stats.update(parsed)
                    logging.debug("Parsed: \n {}".format(json.dumps(parsed)))

                logging.info("Add parser output to reporter")
                report.add_stats(self._alias, suite["name"], stats)

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

    opts, args = getopt.getopt(
        sys.argv[1:], "",
        ["cfg=", "cmake-dir=", "use-saved-data"]
    )

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

    parsers_tbl = {}
    for parser in cfg.get("parsers", []):
        name = parser["name"]
        parsers_tbl[name] = create_parser(name, **parser.get("params", {}))

    printers_tbl = {}
    for printer in cfg.get("printers", []):
        name = printer["name"]
        printers_tbl[name] = create_printer(name, **printer.get("params", {}))

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

            trgt.run(trgt_params, report, parsers_tbl.values(), checker)
            print(checker.results())
    else:
        saved_fn = printers_tbl["json"].outfn()
        with open(saved_fn) as saved:
            report = Report(json.load(saved))

    logging.info("Produce reports")
    for printer in printers_tbl.values():
        printer.print_report(report)


