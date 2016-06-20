import os
import re
import subprocess
import tempfile
import logging
import json
import tabulate

import numpy as np

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

        def parse_tree_info(match):
            tree_depth = int(match.group("tree_depth"))
            self._tree_depth = tree_depth
            if self._tree_depth not in self._context:
                self._context[self._tree_depth] = {}
                self._context[self._tree_depth]["td_time"] = []
                self._context[self._tree_depth]["bp_time"] = []

            self._context[self._tree_depth]["tree_depth"] = tree_depth
            self._context[self._tree_depth]["tree_count"] = int(match.group("tree_count"))

        def parse_top_down_time(match):
            self._context[self._tree_depth]["td_time"] += [int(match.group("td_time"))]

        def parse_bottom_up_time(match):
            self._context[self._tree_depth]["bp_time"] += [int(match.group("bu_time"))]

        def parse_trace_stw_time(match):
            self._context["trace_stw"] += [int(match.group("trace_stw"))]
            self._context["stw_count"][self._i] += 1

        def parse_compact_stw_time(match):
            self._context["compact_stw"] += [int(match.group("compact_stw"))]
            self._context["stw_count"][self._i] += 1

        def parse_full_time(match):
            self._context["full_time"] += [int(match.group("full_time"))]

        def parse_gc_count(match):
            self._context["gc_count"] += [int(match.group("gc_count"))]

        token_spec = {
            "TREE_INFO": {"cmd": parse_tree_info, "re": "Creating (?P<tree_count>\d*) trees of depth (?P<tree_depth>\d*)"},
            "TOP_DOWN" : {"cmd": parse_top_down_time, "re": "\tTop down construction took (?P<td_time>\d*) msec"},
            "BOTTOM_UP": {"cmd": parse_bottom_up_time, "re": "\tBottom up construction took (?P<bu_time>\d*) msec"},
            "TRACE_STW": {"cmd": parse_trace_stw_time, "re": "trace roots stw time = (?P<trace_stw>\d*) microsec"},
            "CMPCT_STW": {"cmd": parse_compact_stw_time, "re": "compact stw time = (?P<compact_stw>\d*) microsec"},
            "FULL_TIME": {"cmd": parse_full_time, "re": "Completed in (?P<full_time>\d*) msec"},
            "GC_COUNT" : {"cmd": parse_gc_count, "re": "Completed (?P<gc_count>\d*) collections"}
        }

        self._scanner = Scanner(token_spec)

    def parse(self, test_output):
        self._context["stw_count"].append(0)
        self._scanner.scan(test_output)
        self._i += 1

    def new_context(self, name):
        self._context = {}
        self._context["name"] = name
        self._context["trace_stw"] = []
        self._context["compact_stw"] = []
        self._context["full_time"] = []
        self._context["stw_count"] = []
        self._context["gc_count"] = []
        self._i = 0

    def result(self):
        res = {}

        res["name"] = self._context["name"]

        res["trace stw mean"] = np.mean(self._context["trace_stw"]) / 1000.0
        res["trace stw std"]  = np.std(self._context["trace_stw"]) / 1000.0
        res["trace stw max"]  = np.max(self._context["trace_stw"]) / 1000.0

        res["compact stw mean"] = np.mean(self._context["compact_stw"]) / 1000.0
        res["compact stw std"]  = np.std(self._context["compact_stw"]) / 1000.0
        res["compact stw max"]  = np.max(self._context["compact_stw"]) / 1000.0

        res["full time mean"] = np.mean(self._context["full_time"])
        res["full time std"]  = np.std(self._context["full_time"])

        res["stw count"] = round(np.mean(self._context["stw_count"]))
        res["gc count"]  = round(np.mean(self._context["gc_count"]))

        return res


class TexTablePrinter:

    def __init__(self, cols):
        self._cols = cols
        self._rows = []

    def add_row(self, row):
        res = []
        for col in self._cols:
            val = row.get(col)
            if val is None:
                continue
            res.append(val)
        self._rows.append(res)

    def print(self):
        return tabulate.tabulate(self._rows, self._cols, tablefmt="latex")


class TestRunner:

    def __init__(self, prj_dir, target, runnable, build_ops=[{}], run_ops=[{}]):
        self._prj_dir = prj_dir
        self._target = target
        self._runnable = runnable
        self._build_ops = build_ops
        self._run_ops = run_ops

    def run(self, parser, printer, nruns):

        for build_op in self._build_ops:
            build_name = build_op.get("name")
            cppflags   = build_op.get("cmake_options")

            logging.info("Build {} with cppflags: {}".format(self._target, cppflags))

            with Builder(self._prj_dir, self._target, cppflags) as builder:
                for run_op in self._run_ops:
                    run_name = run_op.get("name", "")
                    args = run_op.get("args", [])

                    context_name = str(build_name) + run_name
                    parser.new_context(context_name)

                    for n in range(0, nruns):
                        logging.info("Run {} with args: {}".format(self._runnable, args))
                        output = run(builder.build_dir(), self._runnable, args)
                        logging.debug("Output: \n {}".format(output))

                        logging.info("Parse output")
                        parser.parse(output)

                    parsed = parser.result()
                    logging.debug("Parsed: \n {}".format(json.dumps(parsed)))

                    logging.info("Add parsed output to printer")
                    printer.add_row(parsed)

        logging.info("Produce results")
        return printer.print()




PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
BOEHM_TEST_TARGET   = "boehm"
BOEHM_TEST_RUNNABLE = os.path.join("test", "boehm", "boehm")
OUT_FILENAME = "boehm_test_new.tex"

if __name__ == '__main__':

    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    build_ops = [
        # {"name": "T*", "cmake_options": "NO_GC"},
        # {"name": "BoehmGC", "cmake_options": "BDW_GC"},
        # {"name": "shared_ptr", "cmake_options": "SHARED_PTR"},
        {"name": "gc_ptr", "cmake_options": "PRECISE_GC"}
    ]
    cols = ["name",
            "full time mean", "full time std",
            "trace stw mean", "trace stw std", "trace stw max",
            "compact stw mean", "compact stw std", "compact stw max",
            "stw count", "gc count"]

    runner  = TestRunner(PROJECT_DIR, BOEHM_TEST_TARGET, BOEHM_TEST_RUNNABLE, build_ops=build_ops)
    parser  = BoehmTestParser()
    printer = TexTablePrinter(cols)

    result = runner.run(parser, printer, 20)

    outfile = open(OUT_FILENAME, "w")
    outfile.write(result)