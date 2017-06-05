import json


class JSONPrinter:

    def __init__(self):
        pass

    def print_report(self, report, outfn):
        with open(outfn + ".json", "w") as outfile:
            json.dump(report.targets, outfile, indent=4)


def escape_tex(content):
    return content.replace("{", "{{").replace("}", "}}").replace("[[", "{").replace("]]", "}")


class GCTimePlotPrinter:

    def __init__(self):
        with open('gc-time-plot.tex', 'r') as fd:
            self._tpl = escape_tex(fd.read())

    def print_plot(self, targets, suites, report, outfn):
        tpl = escape_tex("""
            \\addplot+[
                [[color]], draw=[[color]], pattern color = [[color]], pattern = [[pattern]],
                error bars/.cd, y dir=both,y explicit,
            ]
            coordinates{
                [[coordinates]]
            };
        """)

        colors = ["brown", "blue", "red", "orange", "violet", "green"]
        patterns = ["horizontal lines", "north west lines", "vertical lines", "dots", "crosshatch", "grid"]

        i = 0
        content = ""
        for suite in suites:
            coordinates = ""
            coordinate  = "({n}, {mean}) +- (0, {std}) [{time} ms]\n"

            n = 1
            for target in targets:
                data = report.suites[suite][target]
                baseline = report.suites["shared_ptr"][target]
                time = data["full_time"]["mean"]
                mean = float(data["full_time"]["mean"]) / float(baseline["full_time"]["mean"])
                std  = float(data["full_time"]["std"]) / float(baseline["full_time"]["mean"])
                coordinates += coordinate.format(n=n, time=int(time), mean=round(mean, 2), std=round(std, 2))
                n += 1

            content += tpl.format(color=colors[i], pattern=patterns[i], coordinates=coordinates)
            i += 1

        xtick = ", ".join(str(i) for i in range(1, len(targets)+1))
        legend = ", ".join(suite.replace("_", "\\_") for suite in suites)
        xticklabels = ", ".join(targets)

        with open(outfn, "w") as outfd:
            outfd.write(self._tpl.format(content=content, legend=legend, xtick=xtick, xticklabels=xticklabels))

    def print_report(self, report, outfn):
        suites1 = ["manual", "shared_ptr", "BDW GC", "BDW GC incremental", "gc_ptr serial", "gc_ptr cms"]
        suites2 = ["shared_ptr", "BDW GC", "BDW GC incremental", "gc_ptr serial", "gc_ptr cms"]
        targets1 = ["gcbench top-down", "gcbench bottom-up", "parallel merge sort"]
        targets2 = ["cord-build", "cord-substr", "cord-flatten"]
        self.print_plot(targets1, suites1, report, outfn + "-1.tex")
        self.print_plot(targets2, suites2, report, outfn + "-2.tex")


class GCHeapPlotPrinter:

    def __init__(self):
        with open('gc-heap-plot.tex', 'r') as fd:
            self._tpl = escape_tex(fd.read())

    def print_report(self, data, outfn):

        def to_Mb(sz):
            return round(float(sz) / (1024 * 1024), 3)

        def iter(data):
            return zip(range(1, len(data)+1), data)

        used = data["heapsize"]
        all  = data["heapextra"]

        used = "\n".join("({}, {})".format(i, to_Mb(sz)) for i, sz in iter(used))
        all  = "\n".join("({}, {})".format(i, to_Mb(sz)) for i, sz in iter(all))
        with open(outfn + ".tex", "w") as outfd:
            outfd.write(self._tpl.format(used=used, all=all))

