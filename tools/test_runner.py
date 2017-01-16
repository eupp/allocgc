import os
import re
import sys
import json
import math
import shutil
import getopt
import logging
import tempfile
import subprocess
import collections


STATS_FULL_TIME_MEAN    = "full time mean"
STATS_FULL_TIME_STD     = "full time std"
STATS_GC_TIME_MEAN      = "gc time mean"
STATS_GC_TIME_STD       = "gc time std"
STATS_STW_TIME_MEAN     = "stw time mean"
STATS_STW_TIME_STD      = "stw time std"
STATS_STW_TIME_MAX      = "stw time max"
STATS_GC_COUNT          = "gc count"


def mean(xs):
    s = 0.0
    for x in xs:
        s += x
    return s / len(xs)


def std(xs):
    return math.sqrt(mean([x**2 for x in xs]) - mean(xs)**2)


def stat_mean(arr, default=float('NaN'), ndigits=None):
    return round(mean(arr), ndigits) if len(arr) > 0 else default


def stat_std(arr, default=float('NaN'), ndigits=None):
    return round(std(arr), ndigits) if len(arr) > 0 else default


def stat_max(arr, default=float('NaN'), ndigits=None):
    return round(max(arr), ndigits) if len(arr) > 0 else default


def call_with_cwd(args, cwd):
    proc = subprocess.Popen(args, cwd=cwd)
    proc.wait()
    assert proc.returncode == 0


def call_output(args, cwd=None, timeout=None):
    proc = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        out, err = proc.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        raise
    return proc.returncode, out.decode("utf-8")


class Build:

    def __init__(self, name, prj_dir, runnable):
        self._name = name
        self._prj_dir = prj_dir
        self._runnable = runnable
        self._tmpdir = tempfile.TemporaryDirectory()

    def __enter__(self):
        self._tmpdir.__enter__()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._tmpdir.__exit__(exc_type, exc_val, exc_tb)

    def name(self):
        return self._name

    def build_dir(self):
        return self._tmpdir.name

    def project_dir(self):
        return self._prj_dir

    def runnable(self):
        return self._runnable


class CMakeBuild(Build):

    _tmpdir = None

    def __init__(self, prj_dir, target, runnable, cmake_ops, *args):
        super(CMakeBuild, self).__init__("cmake", prj_dir, runnable)
        if "run_cmake" in args:
            cmake_cmd = ["cmake", "-DCMAKE_BUILD_TYPE=Release", self._prj_dir] + self._parse_cmake_options(cmake_ops)
            call_with_cwd(cmake_cmd, self.build_dir())
        else:
            self._build_dir = "."
        make_cmd = ["make", target]
        call_with_cwd(make_cmd, self.build_dir())

    def run(self, args):
        runnable = os.path.join(self.build_dir(), self.runnable())
        call_args = [runnable] + args
        return call_output(call_args, cwd=self.build_dir(), timeout=30)

    @staticmethod
    def _parse_cmake_options(flags):
        res = []
        if isinstance(flags, str):
            res.append("-D{}=ON".format(flags))
        elif isinstance(flags, dict):
            for k, v in flags.items():
                if v is None:
                    res.append("-D{}=ON".format(k))
                else:
                    res.append("-D{}={}".format(k, v))
        return res


class MonoBuild(Build):

    def __init__(self, prj_dir, target, runnable, options):
        super(MonoBuild, self).__init__("mono", prj_dir, runnable)
        compile_cmd = ["dmcs", os.path.join(self._prj_dir, runnable + ".cs")]
        call_with_cwd(compile_cmd, self.build_dir())
        dst = os.path.join(self.build_dir(), os.path.dirname(runnable))
        os.makedirs(dst)
        shutil.copy(os.path.join(prj_dir, runnable + ".exe"), dst)

    def run(self, args):
        runnable = os.path.join(self.build_dir(), self.runnable())
        call_args = ["mono", runnable + ".exe"] + args
        return call_output(call_args, cwd=self.build_dir(), timeout=30)


class PythonBuild(Build):

    def __init__(self, prj_dir, target, runnable, options):
        super(PythonBuild, self).__init__("python", prj_dir, runnable)
        dst = os.path.join(self.build_dir(), os.path.dirname(runnable))
        os.makedirs(dst)
        shutil.copy(os.path.join(prj_dir, runnable + ".py"), dst)

    def run(self, args):
        runnable = os.path.join(self.build_dir(), self.runnable())
        call_args = ["python3", runnable + ".py"] + args
        return call_output(call_args, cwd=self.build_dir(), timeout=30)


def build(builder, *args):
    if builder == "cmake":
        return CMakeBuild(*args)
    elif builder == "mono":
        return MonoBuild(*args)
    elif builder == "python":
        return PythonBuild(*args)


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

        token_spec = {
            "FULL_TIME" : {"cmd": parse_full_time, "re": "Completed in (?P<full_time>\d*) ms"},
            "GC_TIME"   : {"cmd": parse_gc_time, "re": "Time spent in gc (?P<gc_time>\d*) ms"},
            "STW_TIME"  : {"cmd": parse_stw_time, "re": "Average pause time (?P<stw_time>\d*) us"},
            "GC_COUNT"  : {"cmd": parse_gc_count, "re": "Completed (?P<gc_count>\d*) collections"}
        }

        self._scanner = Scanner(token_spec)
        self._context = collections.defaultdict(list)

    def parse(self, test_output):
        self._scanner.scan(test_output)

    def result(self):
        return {
            "STATS_FULL_TIME_MEAN"  : stat_mean(self._context["full_time"], ndigits=0),
            "STATS_FULL_TIME_STD"   : stat_std(self._context["full_time"], ndigits=3),
            "STATS_GC_TIME_MEAN"    : stat_mean(self._context["gc_time"], ndigits=0),
            "STATS_GC_TIME_STD"     : stat_std(self._context["gc_time"], ndigits=3),
            "STATS_STW_TIME_MEAN"   : stat_mean(self._context["stw_time"], ndigits=0),
            "STATS_STW_TIME_STD"    : stat_std(self._context["stw_time"], ndigits=3),
            "STATS_STW_TIME_MAX"    : stat_max(self._context["stw_time"], ndigits=0),
            "STATS_GC_COUNT"        : stat_mean(self._context["gc_count"], default=0, ndigits=0)
        }


def create_parser(target):
    if target in ("boehm", "multisize_boehm", "parallel_merge_sort", "pyboehm", "csboehm"):
        return Parser()


class TexTableReporter:

    def __init__(self, outfn):
        self._cols = [
            "name",
            STATS_GC_COUNT,
            STATS_FULL_TIME_MEAN, STATS_FULL_TIME_STD,
            STATS_GC_TIME_MEAN, STATS_GC_TIME_STD,
            STATS_STW_TIME_MEAN, STATS_STW_TIME_STD, STATS_STW_TIME_MAX,
        ]
        self._rows = []
        self._outfn = outfn

    def add_stats(self, row):
        self._rows.append(self._match_columns(row, self._cols))

    def create_report(self):
        import tabulate
        with open(self._outfn, "w") as outfile:
            outfile.write(tabulate.tabulate(self._rows, self._cols, tablefmt="latex"))

    @staticmethod
    def _match_columns(row, cols):
        res = []
        for col in cols:
            res.append(row.get(col))
        return res


def create_reporter(reporter_name, outfn):
    if reporter_name == "tex_table":
        return TexTableReporter(outfn)


class RunChecker:

    def __init__(self, maxn=None, assert_on_fail=False):
        self._runs = collections.defaultdict(lambda: collections.defaultdict(int))
        self._maxn = maxn
        self._assert_on_fail = assert_on_fail

    def check_run(self, target, run_name, rc):
        name = self._full_name(target, run_name)
        self._runs[name]["run count"] += 1
        if self._maxn is not None and self._runs[name]["run count"] > self._maxn:
            assert False, "Too many runs"
        if rc != 0:
            if self._assert_on_fail:
                assert False, "Run failed with error code %d" % rc
            self._runs[name]["failed count"] += 1
        return rc == 0

    def interrupted_run(self, target, run_name):
        name = self._full_name(target, run_name)
        self._runs[name]["run count"] += 1
        self._runs[name]["interrupted count"] += 1

    def get_results(self):
        str = ""
        for k, v in self._runs.items():
            str += k + ":\n"
            str += "\n".join("  %s = %s" % (kk, vv) for (kk, vv) in v.items())
            str += "\n"
        return str

    @staticmethod
    def _full_name(target, run_name):
        return target + " " + run_name


class TestRunner:

    def __init__(self, prj_dir, target, builder, runnable, builds):
        self._prj_dir = prj_dir
        self._target = target
        self._builder = builder
        self._runnable = runnable
        self._builds = builds

    def run(self, nruns, run_checker, reporters=[]):

        for build_cfg in self._builds:
            build_name = build_cfg.get("name")
            cppflags   = build_cfg.get("compile_options")
            options    = build_cfg.get("options", [])

            logging.info("Build {} with cppflags: {}".format(self._target, cppflags))

            with build(self._builder, self._prj_dir, self._target, self._runnable, cppflags, *options) as bld:
                run_ops = build_cfg.get("runtime_options", [{}])
                for run_op in run_ops:
                    run_name = build_name + " " + run_op.get("suffix", "")
                    args = run_op.get("args", "").split()

                    parser = create_parser(self._target)

                    n = 0
                    while n < nruns:
                        logging.info("Run {} with args: {}".format(self._runnable, args))
                        try:
                            rc, output = bld.run(args)
                        except subprocess.TimeoutExpired:
                            logging.info("Interrupted (timeout expired)!")
                            run_checker.interrupted_run(self._target, run_name)
                            continue
                        logging.info("Return code: {}".format(rc))
                        logging.debug("Output: \n {}".format(output))
                        if run_checker.check_run(self._target, run_name, rc):
                            logging.info("Parse output")
                            parser.parse(output)
                            n += 1

                    parsed = parser.result()
                    logging.debug("Parsed: \n {}".format(json.dumps(parsed)))

                    logging.info("Add parsed output to reporter")
                    parsed["name"] = run_name
                    for reporter in reporters:
                        reporter.add_stats(parsed)

        logging.info("Produce reports")
        for reporter in reporters:
            reporter.create_report()

PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")

if __name__ == '__main__':

    print(sys.version)

    opts, args = getopt.getopt(sys.argv[1:], "", ["cfg=", "run-cmake"])
    cli_options = []
    for opt, arg in opts:
        if opt == "--cfg":
            config = arg
        elif opt == "--run-cmake":
            cli_options.append("run_cmake")

    logger = logging.getLogger()
    logger.setLevel(logging.INFO)

    with open(config) as fd:
        cfg = json.load(fd)

    nruns = cfg["nruns"]
    failquick = cfg.get("failquick", False)
    global_options = cfg.get("options", [])

    run_checker = RunChecker(maxn=3*nruns, assert_on_fail=failquick)

    for target_cfg in cfg["targets"]:
        target      = target_cfg["name"]
        builder     = target_cfg["builder"]
        runnable    = target_cfg["runnable"]

        reporters = []
        for reporter_ops in cfg.get("reporters", []):
            reporters.append(create_reporter(reporter_ops["name"], target + reporter_ops["output"]))

        builds = target_cfg.get("builds", cfg["builds"])
        for build_cfg in builds:
            build_name = build_cfg.get("name")
            cppflags   = build_cfg.get("compile_options")
            options    = build_cfg.get("options", [])
            options   += global_options
            options   += cli_options

            logging.info("Build {} with cppflags: {}".format(target, cppflags))

            with build(builder, PROJECT_DIR, target, runnable, cppflags, *options) as bld:
                run_ops = build_cfg.get("runtime_options", [{}])
                for run_op in run_ops:
                    run_name = build_name + " " + run_op.get("suffix", "")
                    args = run_op.get("args", "").split()

                    parser = create_parser(target)

                    n = 0
                    while n < nruns:
                        logging.info("Run {} with args: {}".format(runnable, args))
                        try:
                            rc, output = bld.run(args)
                        except subprocess.TimeoutExpired:
                            logging.info("Interrupted (timeout expired)!")
                            run_checker.interrupted_run(target, run_name)
                            continue
                        logging.info("Return code: {}".format(rc))
                        logging.debug("Output: \n {}".format(output))
                        if run_checker.check_run(target, run_name, rc):
                            logging.info("Parse output")
                            parser.parse(output)
                            n += 1

                    parsed = parser.result()
                    logging.debug("Parsed: \n {}".format(json.dumps(parsed)))

                    logging.info("Add parsed output to reporter")
                    parsed["name"] = run_name
                    for reporter in reporters:
                        reporter.add_stats(parsed)

        logging.info("Produce reports")
        for reporter in reporters:
            reporter.create_report()

    print(run_checker.get_results())
