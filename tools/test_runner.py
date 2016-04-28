import os
import re
import subprocess
import tempfile
import logging
import json

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

    def __init__(self, prj_dir):
        self._prj_dir = prj_dir

    def __enter__(self):
        self._tmpdir = tempfile.TemporaryDirectory()
        self._tmpdir.__enter__()
        cmake_cmd = ["cmake", "-DCMAKE_BUILD_TYPE=Release", self._prj_dir]
        call_with_cwd(cmake_cmd, self._tmpdir.name)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._tmpdir.__exit__(exc_type, exc_val, exc_tb)

    def build(self, target, cppflags={}):
        make_cmd = ["make", self._parse_cppflags(cppflags), target]
        call_with_cwd(make_cmd, self._tmpdir.name)

    def build_dir(self):
        return self._tmpdir.name

    def project_dir(self):
        return self._prj_dir

    @staticmethod
    def _parse_cppflags(flags):
        res = "CPPFLAGS='"
        for k, v in flags.items():
            res += "-D{}={} ".format(k, v)
        return res + "'"


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
            self._context["stw_count"] += 1

        def parse_compact_stw_time(match):
            self._context["compact_stw"] += [int(match.group("compact_stw"))]
            self._context["stw_count"] += 1

        def parse_full_time(match):
            self._context["full_time"] += [int(match.group("full_time"))]

        def parse_gc_count(match):
            self._context["gc_count"] = int(match.group("gc_count"))

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
        self._scanner.scan(test_output)

    def reset(self):
        self._context = {}
        self._context["stw_count"] = 0
        self._context["trace_stw"] = []
        self._context["compact_stw"] = []
        self._context["full_time"] = []
        self._context["gc_count"] = []

    def result(self):
        stat = {}

        stat["trace_stw_mean"] = np.mean(self._context["trace_stw"])
        stat["trace_stw_std"]  = np.std(self._context["trace_stw"])

        stat["compact_stw_mean"] = np.mean(self._context["compact_stw"])
        stat["compact_stw_std"]  = np.std(self._context["compact_stw"])

        stat["full_time_mean"] = np.mean(self._context["full_time"])
        stat["full_time_std"]  = np.std(self._context["full_time"])

        stat["stw_count"] = self._context["stw_count"]
        stat["gc_count"]  = self._context["gc_count"]

        return stat


class TexTablePrinter:

    def add_row(self, row):
        pass


class TestRunner:

    def __init__(self, prj_dir, target, runnable):
        self._builder = Builder(prj_dir)
        self._target = target
        self._runnable = runnable

    def run(self, parser, printer, nruns, cppflags_list=[{}], args_list=[[]]):

        with self._builder as builder:
            for cppflags in cppflags_list:
                logging.info("Build {} with cppflags: {}".format(self._target, cppflags))
                builder.build(self._target, cppflags)

                for args in args_list:
                    parser.reset()
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

        # logging.info("Produce results")
        # return printer.print()


PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
BOEHM_TEST_TARGET   = "boehm_test"
BOEHM_TEST_RUNNABLE = os.path.join("test", "boehm_test", "boehm_test")


if __name__ == '__main__':

    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    runner = TestRunner(PROJECT_DIR, BOEHM_TEST_TARGET, BOEHM_TEST_RUNNABLE)
    parser = BoehmTestParser()
    printer = {}
    runner.run(parser, printer, 1)
