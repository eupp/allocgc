import os
import subprocess
import tempfile

def call_with_cwd(args, cwd):
    proc = subprocess.Popen(args, cwd=cwd)
    proc.wait()
    assert proc.returncode == 0


def parse_cppflags(flags):
    res = "CPPFLAGS='"
    for k, v in flags.items():
        res += "-D{}={} ".format(k, v)
    return res + "'"


def build(prj_dir='', targets=[], cppflags={}):
    tmpdir = tempfile.TemporaryDirectory()
    cmake_cmd = ["cmake", "-DCMAKE_BUILD_TYPE=Release", prj_dir]
    make_cmd = ["make", parse_cppflags(cppflags), " ".join(targets)]
    call_with_cwd(cmake_cmd, tmpdir.name)
    call_with_cwd(make_cmd, tmpdir.name)
    return tmpdir


def run(exe, build_dir):
    exe = os.path.join(build_dir, exe)
    call_with_cwd(exe, build_dir)


if __name__ == '__main__':
    prj_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")
    with build(prj_dir, targets=["boehm_test"]) as build_dir:
        boehm_test = os.path.join("test", "boehm_test", "boehm_test")
        run(boehm_test, build_dir)
