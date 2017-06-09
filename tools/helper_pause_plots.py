import os
import subprocess
import collections

import runner
import parsers
import printers

PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../")


def call(cmd, cwd):
    proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
    try:
        out, err = proc.communicate(timeout=5*60)
    except subprocess.TimeoutExpired:
        proc.kill()
        raise
    print(out)
    return proc.returncode, out.decode("utf-8")


if __name__ == '__main__':

    print(os.getcwd())

    builders = {
        "BDW GC": runner.CMakeBuilder(PROJECT_DIR, "BDW_GC", build_type="Release"),
        "gc_ptr_serial": runner.CMakeBuilder(PROJECT_DIR, "PRECISE_GC_SERIAL", build_type="Release"),
        "gc_ptr_cms": runner.CMakeBuilder(PROJECT_DIR, "PRECISE_GC_CMS", build_type="Release")
    }

    boehm_cmd = "GC_PRINT_STATS=1 {runnable} {args}"

    suites = {
        "BoehmGC": {"builder": builders["BDW GC"], "cmd": boehm_cmd, "parser": parsers.BoehmStatsParser()},
        "BDWGC-incremental": {"builder": builders["BDW GC"], "cmd": boehm_cmd, "args": ["--incremental"], "parser": parsers.BoehmStatsParser()},
        "gc-ptr serial": {"builder": builders["gc_ptr_serial"], "parser": parsers.GCPauseTimeParser()},
        "gc-ptr cms": {"builder": builders["gc_ptr_cms"], "parser": parsers.GCPauseTimeParser()}
    }

    targets = {
        "gcbench top-down": {
            "name": "boehm",
            "runnable": "benchmark/boehm/boehm",
            "suites": ["BoehmGC", "gc-ptr serial", "gc-ptr cms"],
            "params": ["--top-down"]
        },
        "gcbench bottom-up": {
            "name": "boehm",
            "runnable": "benchmark/boehm/boehm",
            "suites": ["BoehmGC", "gc-ptr serial", "gc-ptr cms"],
            "params": ["--bottom-up"]
        },
        "parallel merge sort": {
            "name": "parallel_merge_sort",
            "runnable": "benchmark/parallel_merge_sort/parallel_merge_sort",
            "suites": ["BoehmGC", "gc-ptr serial", "gc-ptr cms"]
        },
        "cord-build": {
            "name": "cord",
            "runnable": "benchmark/cord/cord",
            "suites": ["BoehmGC", "gc-ptr serial", "gc-ptr cms"],
            "params": ["--build", "--len 6"]
        },
        "cord-substr": {
            "name": "cord",
            "runnable": "benchmark/cord/cord",
            "suites": ["BoehmGC", "gc-ptr serial", "gc-ptr cms"],
            "params": ["--substr", "--len 6"]
        },
        "cord-flatten": {
            "name": "cord",
            "runnable": "benchmark/cord/cord",
            "suites": ["BoehmGC", "gc-ptr serial", "gc-ptr cms"],
            "params": ["--flatten", "--len 5"]
        }
    }

    # printer = printers.JSONPrinter()
    #
    # results = collections.defaultdict(list)
    #
    # for name, target in targets.items():
    #     for suite_name in target["suites"]:
    #         suite = suites[suite_name]
    #         build = suite["builder"].build(target["name"])
    #         parser = suite["parser"]
    #         parser.reset()
    #
    #         args = suite.get("args", []) + target.get("params", [])
    #
    #         cmd = suite.get("cmd")
    #         if cmd:
    #             cmd = cmd.format(runnable=target["runnable"], args=" ".join(args))
    #         else:
    #             cmd = "{} {}".format(target["runnable"], " ".join(args))
    #
    #         rc = 1
    #         while rc != 0:
    #             rc, out = call(cmd, build.dirname())
    #         # assert rc == 0
    #         parser.parse(out)
    #
    #         results[name] += [(suite_name, parser.result())]
    #
    # printer.print_report(results, "pauses")

    parser = parsers.JSONParser()
    with open("pauses.json") as fd:
        parser.parse(fd.read())

    results = parser.result()

    printer = printers.GCPauseTimePlotPrinter()
    printer.print_report(parser.result(), "pauses")
