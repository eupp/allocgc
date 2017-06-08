import re
import math
import json
import collections

from functools import partial

def mean(xs):
    s = 0.0
    for x in xs:
        s += x
    return s / len(xs)


def std(xs):
    return math.sqrt(mean([x**2 for x in xs]) - mean(xs)**2)


def stat_mean(arr, default=float('NaN'), ndigits=0):
    return round(mean(arr), ndigits) if len(arr) > 0 else default


def stat_std(arr, default=float('NaN'), ndigits=0):
    return round(std(arr), ndigits) if len(arr) > 0 else default


def stat_max(arr, default=float('NaN'), ndigits=0):
    return round(max(arr), ndigits) if len(arr) > 0 else default


def stat_min(arr, default=float('NaN'), ndigits=0):
    return round(min(arr), ndigits) if len(arr) > 0 else default


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


class JSONParser:

    def __init__(self):
        self.reset()

    def reset(self):
        self._context = {}

    def parse(self, content):
        self._context.update(json.loads(content))

    def result(self):
        from runner import Report
        return Report.from_targets(self._context)


class GCTimeParser:

    def __init__(self):

        def parse_int(key, match):
            self._context[key] += [int(match.group(key))]

        token_spec = {
            "FULL_TIME" : {"cmd": partial(parse_int, "full_time"), "re": "Completed in (?P<full_time>\d*) ms"},
            "GC_TIME"   : {"cmd": partial(parse_int, "gc_time"), "re": "Time spent in gc (?P<gc_time>\d*) ms"},
            "STW_TIME"  : {"cmd": partial(parse_int, "stw_time"), "re": "Average pause time (?P<stw_time>\d*) us"},
            "GC_COUNT"  : {"cmd": partial(parse_int, "gc_count"), "re": "Completed (?P<gc_count>\d*) collections"}
        }

        self._scanner = Scanner(token_spec)
        self.reset()

    def reset(self):
        self._context = collections.defaultdict(list)

    def parse(self, content):
        self._scanner.scan(content)

    def result(self):
        return {
            "full_time": {
                "mean": stat_mean(self._context["full_time"], ndigits=0),
                "std" : stat_std(self._context["full_time"], ndigits=3)
            },
            "gc_time": {
                "mean": stat_mean(self._context["gc_time"], ndigits=0),
                "std" : stat_std(self._context["gc_time"], ndigits=3)
            },
            "stw_time": {
                "mean": self._us_to_ms(stat_mean(self._context["stw_time"]), ndigits=3),
                "std" : self._us_to_ms(stat_std(self._context["stw_time"]), ndigits=3),
                "min" : self._us_to_ms(stat_min(self._context["stw_time"]), ndigits=3),
                "max" : self._us_to_ms(stat_max(self._context["stw_time"]), ndigits=3)
            },
            "gc_count": stat_mean(self._context["gc_count"], default=0, ndigits=0)
        }

    @staticmethod
    def _us_to_ms(us, ndigits=3):
        return round(float(us) / 1000, ndigits)


class GCHeapParser:

    def __init__(self):

        def parse_int(key, match):
            self._context[key] += [int(match.group(key))]

        token_spec = {
            "USED" : {"cmd": partial(parse_int, "used"), "re": "mem used: \s*(?P<used>\d*)"},
            "LIVE" : {"cmd": partial(parse_int, "live"), "re": "mem live: \s*(?P<live>\d*)"},
            "EXTRA": {"cmd": partial(parse_int, "extra"), "re": "mem extra: \s*(?P<extra>\d*)"},
            "GC_START": {"cmd": partial(parse_int, "gctime"), "re": "GC start time: (?P<gctime>\d*)"},
            "GC_FINISH": {"cmd": partial(parse_int, "gctime"), "re": "GC finish time: (?P<gctime>\d*)"}
        }

        self._scanner = Scanner(token_spec)
        self.reset()

    def reset(self):
        self._context = collections.defaultdict(list)

    def parse(self, test_output):
        self._scanner.scan(test_output)

    def result(self):
        return {
            "heapsize" : self._context["live"],
            "heapextra": [(used + extra - live) for used, live, extra in zip(self._context["used"], self._context["live"], self._context["extra"])],
            "gctime": self._context["gctime"]
        }


class GCPauseTimeParser:

    def __init__(self):

        def parse_pause(match):
            pause = int(match.group("pause"))
            pause = round(pause / 1000, 1)
            self._context["pause"] += [pause]

        token_spec = {
            "PAUSE": {"cmd": parse_pause, "re": "pause time: \s*(?P<pause>\d*)"}
        }

        self._scanner = Scanner(token_spec)
        self.reset()

    def reset(self):
        self._context = collections.defaultdict(list)

    def parse(self, test_output):
        self._scanner.scan(test_output)

    def result(self):
        return self._context


class BoehmStatsParser:

    def __init__(self):

        def parse_pause(match):
            pause = int(match.group("pause"))
            if pause == 0:
                pause = 0.5
            self._context["pause"] += [pause]

        def parse_heap(match):
            before_gc = int(match.group("size")) * 1024
            after_gc  = before_gc - int(match.group("freed"))
            self._context["heapsize"] += [0, 0]
            self._context["heapextra"] += [before_gc, after_gc]

        def parse_time(match):
            self._context["gctime"] += [int(match.group("gctime"))]

        token_spec = {
            "HEAP" : {"cmd": parse_heap,  "re": "GC #\d* freed (?P<freed>\d*) bytes, heap (?P<size>\d*) KiB"},
            "PAUSE": {"cmd": parse_pause, "re": "Complete collection took (?P<pause>\d*) msecs"},
            "GC_START": {"cmd": parse_time, "re": "GC start time: (?P<gctime>\d*)"},
            "GC_FINISH": {"cmd": parse_time, "re": "GC finish time: (?P<gctime>\d*)"}
        }

        self._scanner = Scanner(token_spec)
        self.reset()

    def reset(self):
        self._context = collections.defaultdict(list)

    def parse(self, content):
        self._scanner.scan(content)

    def result(self):
        return self._context


class MassifParser:

    def __init__(self):
        def parse_int(key, match):
            self._context[key] += [int(match.group(key))]

        token_spec = {
            "HEAP_SIZE" : {"cmd": partial(parse_int, "heapsize") , "re": "mem_heap_B=(?P<heapsize>\d*)"},
            "HEAP_EXTRA": {"cmd": partial(parse_int, "heapextra"), "re": "mem_heap_extra_B=(?P<heapextra>\d*)"},
            "GC_TIME": {"cmd": partial(parse_int, "gctime"), "re": "time=(?P<gctime>\d*)"}
        }

        self._scanner = Scanner(token_spec)
        self.reset()

    def reset(self):
        self._context = collections.defaultdict(list)

    def parse(self, content):
        self._scanner.scan(content)

    def result(self):
        return self._context