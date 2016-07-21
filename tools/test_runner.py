import os
import re
import subprocess
import tempfile
import logging
import json
import tabulate

import numpy as np

STATS_FULL_TIME_MEAN    = "full time mean"
STATS_FULL_TIME_STD     = "full time std"
STATS_STW_TIME_MEAN     = "stw time mean"
STATS_STW_TIME_STD      = "stw time std"
STATS_STW_TIME_MAX      = "stw time max"
STATS_GC_COUNT          = "gc count"


def stat_mean(arr, default=float('NaN')):
    return np.mean(arr) if len(arr) > 0 else default


def stat_std(arr, default=float('NaN')):
    return np.std(arr) if len(arr) > 0 else default


def stat_max(arr, default=float('NaN')):
    return np.max(arr) if len(arr) > 0 else default


def call_with_cwd(args, cwd):
    proc = subprocess.Popen(args, cwd=cwd)
    proc.wait()
    assert proc.returncode == 0


def call_with_cwd_output(args, cwd):
    proc = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, err = proc.communicate()
    return out.decode("utf-8")


class Builder:

    _tmpdir = None

    def __init__(self, prj_dir, target, cmake_ops):
        self._prj_dir = prj_dir

        self._tmpdir = tempfile.TemporaryDirectory()
        cmake_cmd = ["cmake", "-DCMAKE_BUILD_TYPE=Release", self._prj_dir] + self._parse_cmake_options(cmake_ops)
        make_cmd = ["make", target]

        call_with_cwd(cmake_cmd, self._tmpdir.name)
        call_with_cwd(make_cmd, self._tmpdir.name)

    def __enter__(self):
        self._tmpdir.__enter__()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._tmpdir.__exit__(exc_type, exc_val, exc_tb)

    def build_dir(self):
        return self._tmpdir.name

    def project_dir(self):
        return self._prj_dir

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


def run(build_dir, exe, args):
    exe = os.path.join(build_dir, exe)
    call_args = [exe] + args
    return call_with_cwd_output(call_args, build_dir)


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


class BoehmTestParser:

    def __init__(self):

        def parse_full_time(match):
            self._context["full_time"] += [int(match.group("full_time"))]

        def parse_stw_time(match):
            self._context["stw_time"] += [int(match.group("stw_time"))]

        def parse_gc_count(match):
            self._context["gc_count"] += [int(match.group("gc_count"))]

        token_spec = {
            # "TREE_INFO": {"cmd": parse_tree_info, "re": "Creating (?P<tree_count>\d*) trees of depth (?P<tree_depth>\d*)"},
            # "TOP_DOWN" : {"cmd": parse_top_down_time, "re": "\tTop down construction took (?P<td_time>\d*) msec"},
            # "BOTTOM_UP": {"cmd": parse_bottom_up_time, "re": "\tBottom up construction took (?P<bu_time>\d*) msec"},
            "FULL_TIME": {"cmd": parse_full_time, "re": "Completed in (?P<full_time>\d*) msec"},
            "STW_TIME" : {"cmd": parse_stw_time, "re": "Average pause time (?P<stw_time>\d*) us"},
            "GC_COUNT" : {"cmd": parse_gc_count, "re": "Completed (?P<gc_count>\d*) collections"}
        }

        self._scanner = Scanner(token_spec)

        self._context = {}
        self._context["full_time"] = []
        self._context["stw_time"] = []
        self._context["gc_count"] = []

    def parse(self, test_output):
        self._scanner.scan(test_output)

    def result(self):
        stats = {}
        stats[STATS_FULL_TIME_MEAN] = stat_mean(self._context["full_time"])
        stats[STATS_FULL_TIME_STD]  = stat_std(self._context["full_time"])
        stats[STATS_STW_TIME_MEAN]  = stat_mean(self._context["stw_time"])
        stats[STATS_STW_TIME_STD]   = stat_std(self._context["stw_time"])
        stats[STATS_STW_TIME_MAX]   = stat_max(self._context["stw_time"])
        stats[STATS_GC_COUNT]       = round(stat_mean(self._context["gc_count"], default=0))
        return stats


def create_parser(target):
    if target in ("boehm", "multisize_boehm"):
        return BoehmTestParser()


class TexTableReporter:

    def __init__(self, outfn):
        self._cols = [
            "name",
            STATS_GC_COUNT,
            STATS_FULL_TIME_MEAN, STATS_FULL_TIME_STD,
            STATS_STW_TIME_MEAN, STATS_STW_TIME_STD, STATS_STW_TIME_MAX,
        ]
        self._rows = []
        self._outfn = outfn

    def add_stats(self, row):
        self._rows.append(self._match_columns(row, self._cols))

    def create_report(self):
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


class NumpyDecoder(json.JSONEncoder):

    def default(self, obj):
        if isinstance(obj, np.integer):
            return int(obj)
        elif isinstance(obj, np.floating):
            return float(obj)
        elif isinstance(obj, np.ndarray):
            return obj.tolist()
        else:
            return super(NumpyDecoder, self).default(obj)


class TestRunner:

    def __init__(self, prj_dir, target, runnable, builds):
        self._prj_dir = prj_dir
        self._target = target
        self._runnable = runnable
        self._builds = builds

    def run(self, reporter, nruns):

        for build in self._builds:
            build_name = build.get("name")
            cppflags   = build.get("cmake_options")

            logging.info("Build {} with cppflags: {}".format(self._target, cppflags))
            with Builder(self._prj_dir, self._target, cppflags) as builder:

                run_ops = build.get("runtime_options", [{}])
                for run_op in run_ops:
                    run_name = build_name + run_op.get("suffix", "")
                    args = run_op.get("args", [])

                    parser = create_parser(self._target)

                    for n in range(0, nruns):
                        logging.info("Run {} with args: {}".format(self._runnable, args))
                        output = run(builder.build_dir(), self._runnable, args)
                        logging.debug("Output: \n {}".format(output))

                        logging.info("Parse output")
                        parser.parse(output)

                    parsed = parser.result()
                    logging.debug("Parsed: \n {}".format(json.dumps(parsed, cls=NumpyDecoder)))

                    logging.info("Add parsed output to reporter")
                    parsed["name"] = run_name
                    reporter.add_stats(parsed)

        logging.info("Produce report")
        return reporter.create_report()




PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
CONFIG = "benchcfg.json"

if __name__ == '__main__':

    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    with open(CONFIG) as fd:
        cfg = json.load(fd)

    for target in cfg["targets"]:
        runner = TestRunner(PROJECT_DIR, target["name"], target["runnable"], cfg["builds"])
        for reporter_ops in cfg["reporters"]:
            reporter = create_reporter(reporter_ops["name"], target["name"] + reporter_ops["output"])
            result = runner.run(reporter, cfg["nruns"])