import os
import subprocess
import tempfile

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


if __name__ == '__main__':
    prj_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
    with Builder(prj_dir) as builder:
        builder.build("boehm_test")
        boehm_test_runnable = os.path.join("test", "boehm_test", "boehm_test")
        run(boehm_test_runnable, builder.build_dir())
