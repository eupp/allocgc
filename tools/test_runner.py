import os
import re
import subprocess
import tempfile

from functools import partial


def mean(array_like):
    if not array_like:
        return 0
    return float(sum(array_like)) / len(array_like)


def call_with_cwd(args, cwd):
    proc = subprocess.Popen(args, cwd=cwd)
    proc.wait()
    assert proc.returncode == 0


class Builder:

    _tmpdir = None

    def __init__(self, prj_dir):
        self._tmpdir = tempfile.TemporaryDirectory()
        cmake_cmd = ["cmake", "-DCMAKE_BUILD_TYPE=Release", prj_dir]
        call_with_cwd(cmake_cmd, self._tmpdir.name)

    def __enter__(self):
        self._tmpdir.__enter__()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._tmpdir.__exit__(exc_type, exc_val, exc_tb)

    def build(self, target, cppflags={}):
        make_cmd = ["make", self._parse_cppflags(cppflags), target]
        call_with_cwd(make_cmd, self._tmpdir.name)

    def build_dir(self):
        return self._tmpdir.name

    @staticmethod
    def _parse_cppflags(flags):
        res = "CPPFLAGS='"
        for k, v in flags.items():
            res += "-D{}={} ".format(k, v)
        return res + "'"


def run(exe, build_dir):
    exe = os.path.join(build_dir, exe)
    call_with_cwd(exe, build_dir)


class BoehmTestParser:

    # _tree_re        =
    # _td_time_re     = re.compile()
    _bu_time_re     = re.compile("Bottom up construction took (\d*) msec")
    _total_time_re  = re.compile("Completed in (\d*) msec")
    _total_gc_re    = re.compile("Completed (\d*) collections")

    _tr_stw_re      = re.compile("trace roots stw time = (\d*) mcrs")
    _cmpc_stw_re    = re.compile("compact stw time     = (\d*) mcrs")

    def __init__(self):
        self._parse_actions = {
            "Creating (\d*) trees of depth (\d*)"   : partial(self._parse_creating_tree, self),
            "Top down construction took (\d*) msec" : partial(self._parse_td_construction, self, "top-down"),
            "Bottom up construction took (\d*) msec": partial(self._parse_td_construction, self, "bottom-up"),
            "trace roots stw time = (\d*) mcrs"     : partial(self._parse_stw_time, self, "trace"),
            "compact stw time     = (\d*) mcrs"     : partial(self._parse_stw_time, self, "compact")
        }

    def parse(self, test_output):
        pass

    def _parse_creating_tree(self, match):
        self._tree_depth = int(match.group(1))
        self._results[self._tree_depth] = {}

    def _parse_construction_time(self, constr_type, match):
        total_time = int(match.group(1))
        self._results.append({
            "tree_depth": self._tree_depth,
            "construct_type": constr_type,
            "stw_count": self._stw_count,
            "total_time": total_time,
            "trace_pause_mean": mean(self._trase_stw_times),
            "compact_pause_mean": mean(self._compact_stw_times)
        })

    def _parse_stw_time(self, stw_type, match):
        time = int(match.group(1))
        self._stw_count += 1
        if stw_type == "trace":
            self._trace_stw_times.append(time)
        elif stw_type == "compact":
            self._compact_stw_times.append(time)

if __name__ == '__main__':
    prj_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
    with Builder(prj_dir) as builder:
        builder.build("boehm_test")
        boehm_test_runnable = os.path.join("test", "boehm_test", "boehm_test")
        run(boehm_test_runnable, builder.build_dir())
