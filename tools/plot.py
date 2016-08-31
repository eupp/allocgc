#!/usr/bin/env python
# a bar plot with errorbars

import numpy as np
import matplotlib.pyplot as plt

ft_mean = (330, 847, 219, 291, 1267, 1726, 1490, 2397)
ft_std = (67.72, 79.326, 39.7, 45.455, 328.453, 266.099, 150.134, 633.741)

gct_mean = (0, 0, 0, 0, 19, 23, 11, 143)
gct_std = (0, 0, 0, 0, 10.357, 8.778, 4.71, 31.865)

gc_count = (0, 0, 150, 252, 32, 32, 56, 56)

ind = np.arange(0, 2 * len(ft_mean), 2)  # the x locations for the groups
width = 0.5       # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(ind, ft_mean, width, color='r', yerr=ft_std)
rects2 = ax.bar(ind + width, gct_mean, width, color='b', yerr=gct_std)

# add some text for labels, title and axes ticks
ax.set_ylabel('Time (ms)')
ax.set_title('Parallel merge sort test')
ax.set_xticks(ind + width)
ax.set_xticklabels((#'python', 'C#\n(Mono)',
                    'new/delete', 'shared_ptr', 'BoehmGC', 'BoehmGC\nIncremental',
                    'gc_ptr\nSerial', 'gc_ptr\nCompacting', 'gc_ptr\nIncremental', 'gc_ptr\nIncremental\nCompacting'))

def autolabel(rects):
    # attach some text labels
    for i, rect in enumerate(rects):
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                '%d' % gc_count[i],
                ha='center', va='bottom')

autolabel(rects1)
# autolabel(rects2)

plt.show()