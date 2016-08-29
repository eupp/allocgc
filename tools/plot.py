#!/usr/bin/env python
# a bar plot with errorbars

import numpy as np
import matplotlib.pyplot as plt

means = (15913, 276, 353, 414, 222, 348, 1865, 2663, 2144, 4568)
std = (1187.88, 3.788, 7.249, 5.822, 4.683, 10.016, 10.661, 17.222, 19.077, 21.135)

ind = np.arange(0, 2 * len(means), 2)  # the x locations for the groups
width = 0.5       # the width of the bars

fig, ax = plt.subplots()
rects = ax.bar(ind, means, width, color='r', yerr=std)

# add some text for labels, title and axes ticks
ax.set_ylabel('Time (ms)')
ax.set_title('Boehm test')
ax.set_xticks(ind + width)
ax.set_xticklabels(('python', 'C#\n(Mono)',
                    'new/delete', 'shared_ptr', 'BoehmGC', 'BoehmGC\nIncremental',
                    'gc_ptr\nSerial', 'gc_ptr\nCompacting', 'gc_ptr\nIncremental', 'gc_ptr\nIncremental\nCompacting'))

plt.show()