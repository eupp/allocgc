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
    return out


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
        token_regex = "|".join("(?P<{}>{})".format(tok, spec["re"]) for tok, spec in token_spec.items())
        self._re = re.compile(token_regex)
        self._ts = token_spec

    def scan(self, text):
        match = self._re.match(text)
        while match is not None:
            tok = match.lastgroup
            action = self._ts[tok]["cmd"]
            action(match)
            pos = match.end()
            match = self._re.match(text, pos)


class BoehmTestParser:

    def __init__(self):

        def parse_tree_info(match):
            self._context["tree_count"] = int(match.group("tree_count"))
            self._context["tree_depth"] = int(match.group("tree_depth"))

        def parse_top_down_time(match):
            self._context["td_time"] += [int(match.group("td_time"))]

        def parse_bottom_up_time(match):
            self._context["bp_time"] += [int(match.group("bu_time"))]

        def parse_trace_stw_time(match):
            self._context["trace_stw"] += [int(match.group("trace_stw"))]
            self._context["stw_count"] += 1

        def parse_compact_stw_time(match):
            self._context["compact_stw"] += [int(match.group("compact_stw"))]
            self._context["stw_count"] += 1

        def parse_full_time(match):
            self._context["full_time"] += [int(match.group("full_time"))]

        def parse_gc_count(match):
            self._context["gc_count"] = [int(match.group("gc_count"))]

        token_spec = {
            "TREE_INFO": {"cmd": parse_tree_info, "re": "Creating (?P<tree_count>\d*) trees of depth (?P<tree_depth>\d*)"},
            "TOP_DOWN" : {"cmd": parse_top_down_time, "re": "Top down construction took (?P<td_time>\d*) msec"},
            "BOTTOM_UP": {"cmd": parse_bottom_up_time, "re": "Bottom up construction took (?P<bu_time>\d*) msec"},
            "TRACE_STW": {"cmd": parse_trace_stw_time, "re": "Trace roots stw time = (?P<trace_stw>\d*) microsec"},
            "CMPCT_STW": {"cmd": parse_compact_stw_time, "re": "Compact stw time = (?P<compact_stw>\d*) microsec"},
            "FULL_TIME": {"cmd": parse_full_time, "re": "Completed in (?P<full_time>\d*) msec"},
            "GC_COUNT" : {"cmd": parse_gc_count, "re": "Completed (?P<gc_count>\d*) collections"}
        }

        self._scanner = Scanner(token_spec)

    def parse(self, test_output):
        self._context = {}
        self._context["td_time"] = []
        self._context["bp_time"] = []
        self._context["stw_count"] = 0
        self._context["trace_stw"] = []
        self._context["compact_stw"] = []
        self._context["full_time"] = []
        self._context["gc_count"] = []

        self._scanner.scan(test_output)

        res = {}
        res["td_time_mean"] = np.mean(self._context["td_time"])
        res["td_time_std"]  = np.std(self._context["td_time"])
        res["bp_time_mean"] = np.mean(self._context["bp_time"])
        res["bp_time_std"]  = np.std(self._context["bp_time"])

        res["trace_stw_mean"] = np.mean(self._context["trace_stw"])
        res["trace_stw_std"]  = np.std(self._context["trace_stw"])

        res["compact_stw_mean"] = np.mean(self._context["compact_stw"])
        res["compact_stw_std"]  = np.std(self._context["compact_stw"])

        res["full_time_mean"] = np.mean(self._context["full_time"])
        res["full_time_std"]  = np.std(self._context["full_time"])

        res["stw_count"] = self._context["stw_count"]
        res["gc_count"]  = self._context["gc_count"]

        return res


class TestRunner:

    def __init__(self, prj_dir):
        self._builder = Builder(prj_dir)

    def run(self, target, runnable, cppflags_list, args_list, parser, printer):
        with self._builder as builder:
            for cppflags in cppflags_list:
                logging.info("Build {} with cppflags: {}".format(target, cppflags))
                builder.build(target, cppflags)
                for args in args_list:
                    logging.info("Run {} with args: {}".format(runnable, args))
                    output = run(builder.build_dir(), runnable, args)

                    logging.info("Parse output")
                    logging.debug("Output: \n {}".format(output))
                    parsed = parser.parse(output)
                    logging.debug("Parsed: \n {}".format(json.dumps(parsed)))

                    logging.info("Add parsed output to printer")
                    printer.add_row(parsed)

        logging.info("Produce results")
        return printer.print()


PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")

BOEHM_TEST_TARGET   = "boehm_test"
BOEHM_TEST_RUNNABLE = os.path.join("test", "boehm_test", "boehm_test")


if __name__ == '__main__':
    pass
