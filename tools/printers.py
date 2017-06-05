import json

class JSONPrinter:

    def __init__(self, outfn):
        self._outfn = outfn

    def print_report(self, report):
        with open(self._outfn, "w") as outfile:
            json.dump(report.targets, outfile, indent=4)

    def outfn(self):
        return self._outfn


class GCTimePlotPrinter:

    def __init__(self, outfn):
        self._outfn = outfn
        with open('gc-time-plot.tex', 'r') as fd:
            self._tpl = fd.read()

    def print_report(self, report):
        tpl = """
            \addplot+[
                {color}, draw={color}, pattern color = {color}, pattern = {pattern},
                error bars/.cd, y dir=both,y explicit,
            ]
            coordinates { {coordinates} };
        """

        colors   = []
        patterns = []

        i = 0
        suites = []
        content = ""
        for suite, tbl in report.suites.items():
            suites.append(suite)
            coordinates = ""
            coordinate  = "({n}, {mean}) += (0, {std}) [{time} ms]"

            n = 1
            for target, data in tbl.items():
                baseline = report.suites["shared_ptr"][target]
                time = data["full_time"]["mean"]
                mean = 100 * float(data["full_time"]["mean"]) / float(baseline["full_time"]["mean"])
                mean = 100 * float(data["full_time"]["std"]) / float(baseline["full_time"]["std"])
                coordinates += coordinate.format(n=n, time=time, mean=mean, std=std)
                n += 1

            content += tpl.format(color=colors[i], pattern=patterns[i], coordinates=coordinates)
            i += 1

        legend = "{}".format()

        self._outfn.write(self._tpl.format(content=content))