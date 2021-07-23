def blend(color, factor):
    return int(255 - (255 - color) * factor)


def rgb(red, green, blue, factor=1):
    """Return color as #rrggbb for the given color values."""
    return '#%02x%02x%02x' % (blend(red, factor), blend(green, factor), blend(blue, factor))


colors = {
    'morton': "#FFD200",
    'cuckoo': "#FF7F00",
    'xor': "#CD161C",
    'bloom': "#23507A"
}

from matplotlib.ticker import FuncFormatter


def kilos(x, pos):
    """The two args are the value and tick position"""
    return '%1.0f\\,K' % (x * 1e-3)


def millions(x, pos):
    """The two args are the value and tick position"""
    if x:
        return '%1.0f\\,M' % (x * 1e-6)
    else:
        return '0'


def billions(x, pos):
    """The two args are the value and tick position"""
    if x == 0:
        return '0'
    elif x < 1e8:
        return '%1.0f\\,M' % (x * 1e-6)
    elif x < 1e10:
        return '%1.1f\\,G' % (x * 1e-9)
    else:
        return '%1.0f\\,G' % (x * 1e-9)


def billions2(x, pos):
    """The two args are the value and tick position"""
    if x == 0:
        return '0'
    else:
        return '%1.0f\\,G' % (x * 1e-9)


def perc(x, pos):
    return '%.0f' % (x * 100) + '\%'


def perc2(x, pos):
    return '%.0f' % (x) + '\%'


kilos = FuncFormatter(kilos)
mills = FuncFormatter(millions)
gigs = FuncFormatter(billions)
gigs2 = FuncFormatter(billions2)
percent = FuncFormatter(perc)
percent2 = FuncFormatter(perc2)

markers = {
    'circle': 'o',
    'triangle': '^',
    'square': 's',
    'diamond': 'D'
}


def extract(rows: [dict], index: int, xaxis: str, yaxis: str):
    return rows[index][yaxis]


def extract_time(rows: [dict], index: int, xaxis: str, yaxis: str):
    return rows[index]['real_time']


def extract_throughput(rows: [dict], index: int, xaxis: str, yaxis: str):
    if rows[index]['fixture'] == 'Count':
        return rows[index]['n_elements_lookup'] * 1000 / rows[index]['real_time']
    else:
        return rows[index]['n_elements_build'] * 1000 / rows[index]['real_time']


def extract_speedup(rows: [dict], index: int, xaxis: str, yaxis: str):
    return rows[0]['real_time'] / rows[index]['real_time']


yconverter = {
    'time': extract_time,
    'throughput': extract_throughput,
    'speedup': extract_speedup,
    'DTLB-misses': extract,
    'ITLB-misses': extract,
    'L1D-misses': extract,
    'L1I-misses': extract,
    'LLC-misses': extract,
    'branch-misses': extract,
    'cycles': extract,
    'instructions': extract,
    'task-clock': extract,
    'avg_size': extract,
    'size': extract,
    'bits': extract,
    'retries': extract,
    'fpr': extract
}

xscale = {
    'k': 'linear',
    's': 'linear',
    'n_threads': 'linear',
    'n_partitions': 'log',
    'n_elements_build': 'log',
    'n_elements_lookup': 'log',
    'shared_elements': 'linear'
}

import pandas as pd


def read_benchmark(path: str):
    print(path)
    csv = pd.read_csv(path)

    split_name = csv['name'].apply(lambda x: x.split('/')[0].split('_'))

    csv['k'] = split_name.apply(lambda x: int(x[len(x) - 1]))
    csv['fixture'] = csv['name'].apply(lambda x: x.split('/')[1])
    csv['s'] = csv['name'].apply(lambda x: float(x.split('/')[2]) / 100)
    csv['n_threads'] = csv['name'].apply(lambda x: int(x.split('/')[3]))
    csv['n_partitions'] = csv['name'].apply(lambda x: int(x.split('/')[4]))
    csv['n_elements_build'] = csv['name'].apply(lambda x: int(x.split('/')[5]))
    csv['n_elements_lookup'] = csv['name'].apply(lambda x: int(x.split('/')[6]))
    csv['shared_elements'] = csv['name'].apply(lambda x: float(x.split('/')[7]) / 100)

    csv['name'] = split_name.apply(lambda x: "\_".join(x[0:(len(x) - 1)]))

    data = {}
    for _, row in csv.iterrows():
        data_row = {}
        for label, item in row.iteritems():
            data_row[label] = item

        name = row['name']
        if name not in data:
            data[name] = []
        data[name].append(data_row)

    return data


import matplotlib
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, FuncFormatter
import numpy as np


def latexify(fig_width=None, fig_height=None, columns=1):
    """Set up matplotlib's RC params for LaTeX plotting.
    Call this before plotting a figure.

    Parameters
    ----------
    fig_width : float, optional, inches
    fig_height : float,  optional, inches
    columns : {1, 2}
    """

    # code adapted from http://www.scipy.org/Cookbook/Matplotlib/LaTeX_Examples

    # Width and max height in inches for IEEE journals taken from
    # computer.org/cms/Computer.org/Journal%20templates/transactions_art_guide.pdf

    assert (columns in [1, 2])

    if fig_width is None:
        fig_width = 3.39 if columns == 1 else 6.9  # width in inches

    if fig_height is None:
        golden_mean = (np.sqrt(5) - 1.0) / 2.0  # Aesthetic ratio
        fig_height = fig_width * golden_mean  # height in inches

    MAX_HEIGHT_INCHES = 32.0
    if fig_height > MAX_HEIGHT_INCHES:
        print("WARNING: fig_height too large:" + fig_height +
              "so will reduce to" + MAX_HEIGHT_INCHES + "inches.")
        fig_height = MAX_HEIGHT_INCHES

    params = {'backend': 'ps',
              'pgf.rcfonts': False,
              'axes.labelsize': 7,  # fontsize for x and y labels (was 10)
              'axes.titlesize': 8,
              'font.size': 7,  # was 10
              'legend.fontsize': 7,  # was 8 # was 10
              'legend.handlelength': 1,
              'legend.handletextpad': 0.5,
              'legend.labelspacing': 0.1,  # was 0.1
              'legend.columnspacing': 1.5,
              'legend.borderpad': 0.3,
              'xtick.labelsize': 7,
              'ytick.labelsize': 7,
              'axes.labelpad': 1,
              'axes.titlepad': 3,
              'text.usetex': True,
              'figure.figsize': [fig_width, fig_height],
              'font.family': 'serif',
              'text.latex.preamble': r'\usepackage{amssymb} \usepackage{ifsym} \usepackage[T1]{fontenc} \usepackage{libertine} \usepackage{graphicx}'
              }

    matplotlib.rcParams.update(params)


def format_axes(ax, xscale='linear', yscale='linear'):
    spine_color = 'black'
    for spine in ['top', 'right']:
        ax.spines[spine].set_visible(False)

    for spine in ['left', 'bottom']:
        ax.spines[spine].set_color(spine_color)
        ax.spines[spine].set_linewidth(0.5)

    ax.set_xscale(xscale)
    ax.set_yscale(yscale)

    if yscale == 'log':
        locmaj = matplotlib.ticker.LogLocator(base=10, numticks=12)
        ax.yaxis.set_major_locator(locmaj)
        locmin = matplotlib.ticker.LogLocator(base=10.0, subs=(np.arange(0, 1, 0.1)), numticks=12)
        ax.yaxis.set_minor_locator(locmin)
        ax.yaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())
    else:
        ax.yaxis.set_minor_locator(AutoMinorLocator(n=2))
        ax.yaxis.grid(b=True, which='minor', linestyle=':')

    ax.xaxis.set_ticks_position('bottom')
    ax.xaxis.set_tick_params(direction='out', color=spine_color)

    ax.yaxis.set_ticks_position('left')
    ax.yaxis.set_tick_params(direction='out', color=spine_color)
    ax.yaxis.grid(b=True, which='major')

    ax.tick_params(axis='both', which='major', pad=0.5)

    return ax


def format_axes_imshow(ax):
    spine_color = 'black'

    for spine in ['left', 'bottom', 'top', 'right']:
        ax.spines[spine].set_color(spine_color)
        ax.spines[spine].set_linewidth(0.5)

    ax.yaxis.set_minor_locator(AutoMinorLocator(n=2))

    ax.xaxis.set_ticks_position('bottom')
    ax.xaxis.set_tick_params(direction='out', color=spine_color)

    ax.yaxis.set_ticks_position('left')
    ax.yaxis.set_tick_params(direction='out', color=spine_color)

    ax.tick_params(axis='both', which='major', pad=0.5)
    return ax


def barAxes(ax):
    ax.set_axisbelow(True)


def cm2inch(value):
    return value / 2.54


def reorderLegend(ax=None, order=None, unique=False):
    if ax is None: ax = plt.gca()
    handles, labels = ax.get_legend_handles_labels()
    labels, handles = zip(*sorted(zip(labels, handles), key=lambda t: t[0]))  # sort both labels and handles by labels
    if order is not None:  # Sort according to a given list (not necessarily complete)
        keys = dict(zip(order, range(len(order))))
        labels, handles = zip(*sorted(zip(labels, handles), key=lambda t, keys=keys: keys.get(t[0], np.inf)))
    if unique:
        labels, handles = zip(*unique_everseen(zip(labels, handles), key=labels))  # Keep only the first of each handle
    return handles, labels


def unique_everseen(seq, key=None):
    seen = set()
    seen_add = seen.add
    return [x for x, k in zip(seq, key) if not (k in seen or seen_add(k))]


import os


def savefig(path, padding=0):
    dir = os.path.dirname(path)
    if not os.path.exists(dir):
        os.makedirs(dir)
    plt.savefig(path + ".pdf", bbox_inches='tight', pad_inches=padding, dpi=300)
    plt.savefig(path + ".pgf", bbox_inches='tight', pad_inches=padding, dpi=300)


import math
from matplotlib.colors import LogNorm


def analyzeCount(data, dataSize):
    if dataSize not in b_count_map:
        b_count_map[dataSize] = {}
    for name_orig in data.keys():
        name = name_orig.replace('\\', '').replace("Fuse", "Xor_Fuse")
        if name not in b_count_map[dataSize]:
            b_count_map[dataSize][name] = {}
        for benchmark in data[name_orig]:
            k = benchmark['k']
            s = round(benchmark['s'] * benchmark['k'])
            n_elements_build = benchmark['n_elements_build']
            n_elements_lookup = benchmark['n_elements_lookup']
            size = benchmark['size']
            f = benchmark['failures']
            time = benchmark['duration'] / (n_elements_lookup / 1000000)
            throughput = n_elements_lookup / benchmark['duration'] * 1000
            dtlb = benchmark['DTLB-misses'] / n_elements_lookup
            l1d = benchmark['L1D-misses'] / n_elements_lookup
            llc = benchmark['LLC-misses'] / n_elements_lookup
            fpr = benchmark['fpr']
            p = math.log(benchmark['n_partitions'], 2)
            if f == 0 and not (benchmark['error_occurred'] == True):
                if not k in b_count_map[dataSize][name]:
                    b_count_map[dataSize][name][k] = {}
                if not s in b_count_map[dataSize][name][k] or throughput > b_count_map[dataSize][name][k][s]['throughput']:
                    b_count_map[dataSize][name][k][s] = {'k': k, 'size': size, 'time': time, 'fpr': fpr, 'dtlb': dtlb, 'l1d': l1d, 'llc': llc, 'p': p, 'throughput': throughput}


def analyzeConstruct(data, dataSize):
    if dataSize not in b_construct_map:
        b_construct_map[dataSize] = {}
    for name_orig in data.keys():
        name = name_orig.replace('\\', '').replace("Fuse", "Xor_Fuse")
        if name not in b_construct_map[dataSize]:
            b_construct_map[dataSize][name] = {}
        for benchmark in data[name_orig]:
            k = benchmark['k']
            s = round(benchmark['s'] * benchmark['k'])
            n_elements_build = benchmark['n_elements_build']
            size = benchmark['size']
            f = benchmark['failures']
            time = benchmark['duration'] / (n_elements_build / 1000000)
            throughput = n_elements_build / benchmark['duration'] * 1000
            dtlb = benchmark['DTLB-misses'] / n_elements_build
            l1d = benchmark['L1D-misses'] / n_elements_build
            llc = benchmark['LLC-misses'] / n_elements_build
            # Connect to other FPR
            if name in b_count_map[dataSize] and k in b_count_map[dataSize][name] and s in b_count_map[dataSize][name][k]:
                fpr = b_count_map[dataSize][name][k][s]["fpr"]
            else:
                fpr = 0

            p = math.log(benchmark['n_partitions'], 2)
            if f == 0 and not (benchmark['error_occurred'] == True):
                if not k in b_construct_map[dataSize][name]:
                    b_construct_map[dataSize][name][k] = {}
                if not s in b_construct_map[dataSize][name][k] or throughput > b_construct_map[dataSize][name][k][s]['throughput']:
                    b_construct_map[dataSize][name][k][s] = {'k': k, 'size': size, 'time': time, 'fpr': fpr, 'dtlb': dtlb, 'l1d': l1d, 'llc': llc, 'p': p, 'throughput': throughput}


benchmarks = {"size": ["10k", "1m", "100m"], "filters": ["bloom", "xor", "cuckoo", "morton", "fuse"]}
base_path = '../benchmark/paper/experiments/'
b_count_map = {}
b_construct_map = {}

for name in benchmarks["filters"]:
    for size in benchmarks["size"]:
        filename = base_path + '{0}/{0}_{1}_{2}.csv'.format(name, "count", size)
        analyzeCount(read_benchmark(filename), size)
        if size != "10k":
            filename = base_path + '{0}_part/{0}_{1}_{2}.csv'.format(name, "count_part", size)
        analyzeCount(read_benchmark(filename), size)

benchmarks = {"size": ["1m", "100m"], "filters": ["bloom", "xor", "cuckoo", "fuse"]}
for name in benchmarks["filters"]:
    for size in benchmarks["size"]:
        filename = base_path + '{0}_part/{0}_{1}_{2}.csv'.format(name, "construct_mt_part", size)
        analyzeConstruct(read_benchmark(filename), size)

from matplotlib.lines import Line2D

datamap = {}
aggmap = {}

for size in b_count_map:
    mapUsed = b_count_map[size]
    if size not in datamap:
        datamap[size] = {"fpr": [], "size": [], "data": []}
    for name in mapUsed:
        for k in mapUsed[name]:
            for s in mapUsed[name][k]:
                point = mapUsed[name][k][s]
                point["name"] = name
                point["k"] = k
                point["s"] = s
                if point["fpr"] == 0:
                    continue
                datamap[size]["fpr"].append(point["fpr"])
                datamap[size]["size"].append(point["s"])
                datamap[size]["data"].append(point)


def binNumber(x, binsPer10):
    return -round(math.log10(x) * binsPer10)


binsPer10 = 3
for size in datamap:
    d = datamap[size]
    fprs = d["fpr"]
    sizes = d["size"]
    minFpr = -math.floor(math.log10(min(fprs)) * binsPer10)
    agg = [[[] for x in range(max(sizes) + 1)] for y in range(minFpr + 1)]
    for i in range(len(fprs)):
        binNum = binNumber(fprs[i], binsPer10)
        for j in range(binNum + 1):
            pos = sizes[i]
            agg[j][pos].append(d["data"][i])
    aggmap[size] = agg

datamap_construct = {}
aggmap_construct = {}

for size in b_construct_map:
    mapUsed = b_construct_map[size]
    if size not in datamap_construct:
        datamap_construct[size] = {"fpr": [], "size": [], "data": []}
    for name in mapUsed:
        for k in mapUsed[name]:
            for s in mapUsed[name][k]:
                point = mapUsed[name][k][s]
                point["name"] = name
                point["k"] = k
                point["s"] = s
                if point["fpr"] == 0:
                    continue
                datamap_construct[size]["fpr"].append(point["fpr"])
                datamap_construct[size]["size"].append(point["s"])
                datamap_construct[size]["data"].append(point)


def binNumber(x, binsPer10):
    return -round(math.log10(x) * binsPer10)


binsPer10 = 3
for size in datamap_construct:
    d = datamap_construct[size]
    fprs = d["fpr"]
    sizes = d["size"]
    minFpr = -math.floor(math.log10(min(fprs)) * binsPer10)
    agg = [[[] for x in range(max(sizes) + 1)] for y in range(minFpr + 1)]
    for i in range(len(fprs)):
        binNum = binNumber(fprs[i], binsPer10)
        for j in range(binNum + 1):
            pos = sizes[i]
            agg[j][pos].append(d["data"][i])
    aggmap_construct[size] = agg

import matplotlib.ticker as ticker
from matplotlib.colors import ListedColormap
from matplotlib import cm
import math
import copy
from matplotlib.path import Path
import matplotlib.patches as patches


def plotFPR(ax, data, size):
    preparePlot(ax, size)
    maxVal = (max([max([y if y > 0 else 0 for y in x]) for x in data]))
    minVal = (min([min([y if y < 0 else 0 for y in x]) for x in data]))
    if minVal < 0:
        mapWithBad = copy.copy(cm.get_cmap('coolwarm'))
        mapWithBad.set_bad("w")
        return ax.imshow(data, cmap=mapWithBad, vmax=max(maxVal, -minVal), vmin=min(minVal, -maxVal))
    else:
        return ax.imshow(data, cmap="plasma")


def preparePlot(ax, size):
    def logPrintFormat(x, pos):
        return "$10^{{\\scalebox{{0.75}}[1.0]{{-}}{}}}$".format(round(x / 3))

    ax.yaxis.set_major_formatter(FuncFormatter(logPrintFormat))
    ax.yaxis.set_major_locator(ticker.MultipleLocator(3))
    ax.set_ylabel("False-positive rate $\\varepsilon$")
    minorSteps = [3 * (1 - math.log10(x)) for x in range(2, 10)]
    steps = [3 * y + x for x in minorSteps for y in range(8)]
    ax.yaxis.set_minor_locator(ticker.FixedLocator(steps))
    ax.set_ylim(bottom=20.5, top=2.5)

    if size == "10k":
        ax.set_xlabel("Filter size $m$ [KB]")

        def sizeFormat(x, pos):
            return round(x * 1.25)
    else:
        ax.set_xlabel("Filter size $m$ [MB]")
        if size == "1m":
            def sizeFormat(x, pos):
                return round(x * 125 / 1000, 1)
        else:
            def sizeFormat(x, pos):
                return round(x * 125 / 10)

    ax.xaxis.set_major_formatter(FuncFormatter(sizeFormat))
    ax.xaxis.set_major_locator(ticker.MultipleLocator(4))
    ax.xaxis.set_minor_locator(ticker.MultipleLocator(2))
    ax.set_xlim(left=4.5, right=25.5)


def mapData(data, mapperFun):
    return [[mapperFun(y) for y in x] for x in data]


def drawSeparatorLine(ax, size, color="w"):
    def bestName(ps):
        best = getBest(ps)
        return best["name"][0] if best else ""

    data = mapData(aggmap[size], bestName)
    verts = [[0, 0]]
    codes = [Path.MOVETO]
    lasty = 0
    for i in range(len(data[0])):
        y = (robustMax([j for j in range(len(data)) if "B" in data[j][i]]))
        if lasty != y:
            verts.append([i - 0.5, y + 0.5])
            lasty = y
            codes.append(Path.LINETO)
        verts.append([i + 0.5, y + 0.5])
        codes.append(Path.LINETO)
    path = Path(verts, codes)

    patch = patches.PathPatch(path, facecolor='none', edgecolor=color, lw=.75)
    ax.add_patch(patch)


def drawScatterSymbols(ax, data, info):
    best = {}
    for n in info:
        best[n] = [[], []]

    # find the best bloom
    for i in range(len(data)):
        for j in range(len(data[i])):
            if data[i][j]:
                best[data[i][j]][0].append(j)
                best[data[i][j]][1].append(i)
    legend_elements = []
    for f in best:
        legend_elements.append(Line2D([0], [0], marker=info[f][0], color='k', label=info[f][1], linestyle='None', markersize=3, linewidth=0.1))
        ax.scatter(best[f][0], best[f][1], color='w', marker=info[f][0], s=3, linewidths=0.3)
    if len(legend_elements):
        ax.legend(handles=legend_elements, loc='lower left', frameon=False, borderaxespad=0)
    ax.set_title(size)


def genericMapper(vs):
    fprs = robustMax([v["fpr"] for v in vs])
    sizes = robustMin([v["size"] for v in vs])
    out = [fprs, sizes]
    for name in ["Bloom", "Cuckoo", "Morton", "Xor"]:
        out.append(robustMax([v["throughput"] for v in vs if name in v["name"]]))
    return out


def fprSlice(d, x):
    return d[x]


def mapFprSlice(d, x, mapper):
    return [mapper(i) for i in fprSlice(d, x)]


def sizeSlice(d, x):
    return [d[s][x] for s in range(len(d))]


def mapSizeSlice(d, x, mapper):
    return [mapper(i) for i in sizeSlice(d, x)]


def robustMax(vs):
    return max(vs) if len(vs) else float("nan")


def robustMin(vs):
    return min(vs) if len(vs) else float("nan")


def getBest(vs):
    best = {}
    for p in vs:
        if not best or p["throughput"] > best["throughput"]:  # and "Part" not in p["name"] and "AVX" not in p["name"]:
            best = p
    return best


def getBestIf(vs, cond):
    best = {}
    for p in vs:
        if cond(p) and (not best or p["throughput"] > best["throughput"]):  # and "Part" not in p["name"] and "AVX" not in p["name"]:
            best = p
    return best


from matplotlib.patches import Patch, Rectangle


def mapBestTpName(ps):
    tp = 0
    name = ""
    for p in ps:
        if p["throughput"] > tp:
            tp = p["throughput"]
            name = p["name"][0]
    mapping = {"B": 0, "M": 2, "C": 1, "X": 3}
    return mapping[name] if tp else float("nan")


info = ["Bloom", "Cuckoo", "Morton", "Xor"]
names = mapData(aggmap["100m"], mapBestTpName)
latexify(cm2inch(8.5), cm2inch(3.75), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "100m")
newcmp = ListedColormap([colors[x.lower()] for x in info])
ax1.imshow(names, cmap=newcmp, aspect="auto")
ax1.set_aspect("equal")
legend_elements = []
for f in info:
    legend_elements.append(Patch(facecolor=colors[f.lower()], label=f))
ax1.legend(handles=legend_elements, loc='lower left', handlelength=1, handleheight=1, frameon=False, borderaxespad=0)
for y in range(2, 25):
    ax1.axhline(y + 0.5, color="w", linewidth=0.5)
for x in range(2, 25):
    ax1.axvline(x + 0.5, color="w", linewidth=0.5)
ax1.set_title("Best Performing Filter")
drawSeparatorLine(ax1, "100m")
# Create a Rectangle patch
ax1.add_patch(Rectangle((22.5, 2.5), 1, 16, linewidth=1, edgecolor='k', facecolor='none', zorder=10, clip_on=False))

ax2 = fig.add_subplot(spec[0, 1])
fprs = np.array(mapSizeSlice(aggmap["100m"], 23, genericMapper))
format_axes(ax2, "log", "linear")
ax2.invert_xaxis()
for i in reversed(range(4)):
    i2 = i + 2
    ax2.plot(fprs[:, 0], fprs[:, i2], color=colors[info[i].lower()])
#  ax2.text(fprs[4,0], fprs[6,i2]+(1e7 if i in [1,2] else -1.5e7), "\\textbf{{{}}}".format(info[i]), horizontalalignment='left', verticalalignment=("bottom" if i in [1,2] else "top"), fontsize=6, color=colors[info[i].lower()])

ax2.set_ylim(0, 1020e6)
ax2.set_xlim(left=1e-1)
ax2.set_title("250\,MB Filter Size")
ax2.set_ylabel("Throughput [Keys/s]")
ax2.set_xlabel("False-positive rate $\\varepsilon$")


def logPrintFormat(x, pos):
    return "$10^{{\\scalebox{{0.75}}[1.0]{{-}}{}}}$".format(round(-math.log10(x)))


ax2.xaxis.set_major_formatter(FuncFormatter(logPrintFormat))
ax2.yaxis.set_major_formatter(FuncFormatter(millions))

locmaj = matplotlib.ticker.LogLocator(base=10, numticks=12)
ax2.xaxis.set_major_locator(locmaj)
locmin = matplotlib.ticker.LogLocator(base=10.0, subs=(np.arange(0, 1, 0.1)), numticks=12)
ax2.xaxis.set_minor_locator(locmin)
ax2.xaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())

savefig("./pdf/introduction/skyline", 0.03)

from matplotlib.patches import Patch, Rectangle


def nonpartMapper(vs):
    fprs = robustMax([v["fpr"] for v in vs])
    sizes = robustMin([v["size"] for v in vs])
    out = [fprs, sizes]
    for name in ["Bloom", "Cuckoo", "Morton", "Xor"]:
        out.append(robustMax([v["throughput"] for v in vs if name in v["name"] and "Part" not in v["name"]]))
    return out


def partMapper(vs):
    fprs = robustMax([v["fpr"] for v in vs])
    sizes = robustMin([v["size"] for v in vs])
    out = [fprs, sizes]
    for name in ["Bloom", "Cuckoo", "Morton", "Xor"]:
        out.append(robustMax([v["throughput"] for v in vs if name in v["name"] and "Part" in v["name"]]))
    return out


info = ["Bloom", "Cuckoo", "Morton", "Xor"]

latexify(cm2inch(8.5), cm2inch(3.5), 2)
fig = plt.figure(constrained_layout=False)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes(ax1)
fprs = np.array(mapFprSlice(aggmap["1m"], 9, genericMapper))
fprsnon = np.array(mapFprSlice(aggmap["1m"], 9, nonpartMapper))
fprsyes = np.array(mapFprSlice(aggmap["1m"], 9, partMapper))
for i in reversed(range(4)):
    i2 = i + 2
    ax1.plot(fprsnon[:, 1], fprsnon[:, i2], color=colors[info[i].lower()], linestyle='dashed')
#    ax1.plot(fprsyes[:,1], fprsyes[:,i2], color=colors[info[i].lower()], linestyle='dotted', linewidth=1.5)
for i in reversed(range(4)):
    i2 = i + 2
    ax1.plot(fprs[:, 1], fprs[:, i2], color=colors[info[i].lower()], label=info[i])
ax1.set_title("$10^{\\scalebox{0.75}[1]{-}3}$ False-positive rate")
ax1.set_ylabel("Throughput [Keys/s]")
ax1.set_ylim(bottom=0, top=2.6e9)
ax1.set_xlabel("Filter size $m$ [MB]")
ax1.yaxis.set_major_formatter(FuncFormatter(billions))
ax1.xaxis.set_major_formatter(FuncFormatter(lambda x, pos: round(x / 1e6, 1)))
ax1.xaxis.set_minor_locator(ticker.MultipleLocator(base=5e5))

ax2 = fig.add_subplot(spec[0, 1])
fprs = np.array(mapSizeSlice(aggmap["1m"], 24, genericMapper))
fprsnon = np.array(mapSizeSlice(aggmap["1m"], 24, nonpartMapper))
format_axes(ax2, "log", "linear")
ax2.invert_xaxis()
for i in reversed(range(4)):
    i2 = i + 2
    ax2.plot(fprsnon[:, 0], fprsnon[:, i2], color=colors[info[i].lower()], linestyle='dashed')
for i in reversed(range(4)):
    i2 = i + 2
    ax2.plot(fprs[:, 0], fprs[:, i2], color=colors[info[i].lower()], label=info[i])
ax2.set_ylim(bottom=0, top=2.6e9)
ax2.set_xlim(left=1e-1)
ax2.set_title("3.0\,MB Filter Size")
ax2.set_ylabel("Throughput [Keys/s]")
ax2.set_xlabel("False-positive rate $\\varepsilon$")


def logPrintFormat(x, pos):
    return "$10^{{\\scalebox{{0.75}}[1.0]{{-}}{}}}$".format(round(-math.log10(x)))


ax2.xaxis.set_major_formatter(FuncFormatter(logPrintFormat))
ax2.yaxis.set_major_formatter(FuncFormatter(billions))

locmaj = matplotlib.ticker.LogLocator(base=10, numticks=12)
ax2.xaxis.set_major_locator(locmaj)
locmin = matplotlib.ticker.LogLocator(base=10.0, subs=(np.arange(0, 1, 0.1)), numticks=12)
ax2.xaxis.set_minor_locator(locmin)
ax2.xaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())
ax2.set_ylabel("")
ax2.set_yticklabels([])
handles, labels = ax2.get_legend_handles_labels()
allfig = plt.gcf()
allfig.legend(reversed(handles), ["Bloom filter", "Cuckoo filter", "Morton filter", "Xor filter"], ncol=4, bbox_to_anchor=(0.5, 1.05), loc='upper center', borderaxespad=0, frameon=False,
              columnspacing=1)

plt.tight_layout()
savefig("./pdf/evaluation/slices", 0)


def mapBestTp(ps):
    return robustMax([p["throughput"] for p in ps if "Bloom" in p["name"]])


def mapBestTpName(ps):
    best = getBestIf(ps, lambda p: "Bloom" in p["name"])
    if not best:
        return ""
    elif "Blocked512" in best["name"]:
        return "5"
    else:
        return best["name"][6] if best else ""


info = {"N": ["H", 'Na\\"ive'], "B": ["s", "R-Blocked"], "5": ["$\\square$", "CL-Blocked"], "S": ["v", "Sectorized"], "G": ["^", "Cache-Sect."]}

latexify(cm2inch(8.5), cm2inch(4), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "1m")
data = mapData(aggmap["1m"], mapBestTp)
ax1.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=5e8, vmax=4e9))
names = mapData(aggmap["1m"], mapBestTpName)
drawScatterSymbols(ax1, names, info)
drawSeparatorLine(ax1, "1m")
ax1.set_aspect("equal")
# ax1.set_ylim(bottom=17)
ax1.set_title("1\,M Elements")

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2, "100m")
data = mapData(aggmap["100m"], mapBestTp)
imshow = ax2.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=5e8, vmax=4e9))
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")


# ax2.set_ylim(bottom=17)
def specialFormat(x, _):
    if x >= 1e9 or x == 6e8:
        return "{}\,G".format(round(x / 1e9, 1))


cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, format=ticker.FuncFormatter(specialFormat))

cbar.ax.yaxis.set_minor_formatter(ticker.FuncFormatter(specialFormat))
# cbar.ax.set_yticklabels([0 for i in cbar.get_ticks()]) # set ticks of your format

names = mapData(aggmap["100m"], mapBestTpName)
drawScatterSymbols(ax2, names, info)
drawSeparatorLine(ax2, "100m")
ax2.set_title("100\,M Elements")
cbar.set_label('Throughput [Keys/s]')
savefig("./pdf/evaluation/bestVariant", 0.03)


def mapBestTp(ps):
    return robustMax([p["throughput"] for p in ps])


def mapBestTpName(ps):
    best = getBest(ps)
    return best["name"][0] if best else ""


info = {"B": ["$\\circ$", "Bloom"], "C": ["$\\vee$", "Cuckoo"], "M": ["$\\wedge$", "Morton"], "X": ["$\\times$", "Xor"]}

vmin = 1e8
vmax = 1e9

latexify(cm2inch(8.5), cm2inch(4), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "1m")
data = mapData(aggmap_construct["1m"], mapBestTp)
ax1.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=vmin, vmax=vmax))
names = mapData(aggmap_construct["1m"], mapBestTpName)
drawScatterSymbols(ax1, names, info)
drawSeparatorLine(ax1, "1m")
ax1.set_aspect("equal")
# ax1.set_ylim(bottom=17)
ax1.set_title("1\,M Elements")

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2, "100m")
data = mapData(aggmap_construct["100m"], mapBestTp)
imshow = ax2.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=vmin, vmax=vmax))
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")
# ax2.set_ylim(bottom=17)
cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, format=billions)


# cbar.ax.set_yticklabels([0 for i in cbar.get_ticks()]) # set ticks of your format
def specialFormat(x, _):
    if str(x)[0] in ["2", "3", "5", "7"]:
        return "{}\,G".format(round(x / 1e9, 1))


cbar.ax.yaxis.set_minor_formatter(ticker.FuncFormatter(specialFormat))

names = mapData(aggmap_construct["100m"], mapBestTpName)
drawScatterSymbols(ax2, names, info)
drawSeparatorLine(ax2, "100m")
ax2.set_title("100\,M Elements")
cbar.set_label('Throughput [Keys/s]')
savefig("./pdf/evaluation/bestBuild", 0.03)


def mapBestTp(ps):
    return robustMax([p["throughput"] for p in ps])


def mapBestTpName(ps):
    best = getBest(ps)
    return best["name"][0] if best else ""


info = {"B": ["$\\circ$", "Bloom"], "C": ["$\\vee$", "Cuckoo"], "M": ["$\\wedge$", "Morton"], "X": ["$\\times$", "Xor"]}

vmin = 1e8
vmax = 1e9

latexify(cm2inch(8.5), cm2inch(4), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "1m")
data = mapData(aggmap_construct["1m"], mapBestTp)
ax1.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=vmin, vmax=vmax))
names = mapData(aggmap_construct["1m"], mapBestTpName)
drawScatterSymbols(ax1, names, info)
drawSeparatorLine(ax1, "1m")
ax1.set_aspect("equal")
# ax1.set_ylim(bottom=17)
ax1.set_title("1\,M Elements")

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2, "100m")
data = mapData(aggmap_construct["100m"], mapBestTp)
imshow = ax2.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=vmin, vmax=vmax))
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")
# ax2.set_ylim(bottom=17)
cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, format=billions)


# cbar.ax.set_yticklabels([0 for i in cbar.get_ticks()]) # set ticks of your format
def specialFormat(x, _):
    if str(x)[0] in ["2", "3", "5", "7"]:
        return "{}\,G".format(round(x / 1e9, 1))


cbar.ax.yaxis.set_minor_formatter(ticker.FuncFormatter(specialFormat))

names = mapData(aggmap_construct["100m"], mapBestTpName)
drawScatterSymbols(ax2, names, info)
drawSeparatorLine(ax2, "100m")
ax2.set_title("100\,M Elements")
cbar.set_label('Throughput [Keys/s]')
savefig("./pdf/evaluation/bestBuild", 0.03)


def mapBestTp(ps):
    return robustMax([p["throughput"] for p in ps])


def mapBestTpName(ps):
    best = getBest(ps)
    return best["name"][0] if best else ""


info = {"B": ["$\\circ$", "Bloom"], "C": ["$\\vee$", "Cuckoo"], "M": ["$\\wedge$", "Morton"], "X": ["$\\times$", "Xor"]}

latexify(cm2inch(8.5), cm2inch(4), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "10k")
data = mapData(aggmap["10k"], mapBestTp)
m = 0
for i in data:
    for j in i:
        if j > m: m = j
print(m)
ax1.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=7e8, vmax=6.5e9))
names = mapData(aggmap["10k"], mapBestTpName)
drawScatterSymbols(ax1, names, info)
drawSeparatorLine(ax1, "10k")
ax1.set_aspect("equal")
# ax1.set_ylim(bottom=17)
ax1.set_title("10\,K Elements")

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2, "1m")
data = mapData(aggmap["1m"], mapBestTp)
imshow = ax2.imshow(data, cmap="plasma", aspect="auto", norm=LogNorm(vmin=7e8, vmax=6.5e9))
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")


# ax2.set_ylim(bottom=17)
def specialFormat(x, _):
    if x >= 1e9 and str(x)[0] != "5" and str(x)[0] != "3":
        return "{}\,G".format(round(x / 1e9))


cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, format=ticker.FuncFormatter(specialFormat))
cbar.ax.yaxis.set_minor_formatter(ticker.FuncFormatter(specialFormat))
# for text in cbar.ax.get_yminorticklabels() + cbar.ax.get_yticklabels():
#    text.set_rotation(90)
#    text.set_va("center")
#    text.set_ha("left")

names = mapData(aggmap["1m"], mapBestTpName)
drawScatterSymbols(ax2, names, info)
drawSeparatorLine(ax2, "1m")
ax2.set_title("1\,M Elements")
cbar.set_label('Throughput [Keys/s]')
savefig("./pdf/evaluation/bestLookup", 0.03)


def mapBestTp(ps):
    tpBest = getBestIf(ps, lambda p: "Part" in p["name"])
    tpNonBest = getBestIf(ps, lambda p: "Part" not in p["name"])
    # if tpBest and tpBest["s"] == 4:
    #    print(f"{tpBest} vs {tpNonBest}")
    tpPart = tpBest["throughput"] if tpBest else tpBest
    tpNonPart = tpNonBest["throughput"] if tpNonBest else tpNonBest
    if tpPart and tpNonPart and tpNonBest["name"][0] in [p["name"][0] for p in ps if "Part" in p["name"]]:
        return tpPart / tpNonPart - 1 if tpPart > tpNonPart else -tpNonPart / tpPart + 1
    else:
        return float("nan")


latexify(cm2inch(8.5), cm2inch(4), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "1m")
data = mapData(aggmap["1m"], mapBestTp)
ax1.imshow(data, cmap="RdBu", aspect="auto", vmin=-1.75, vmax=1.75)
drawSeparatorLine(ax1, "1m", "gray")
ax1.set_aspect("equal")
# ax1.set_ylim(bottom=17)
ax1.set_title("1\,M Elements")

mi = None
ma = None
for i in data:
    for j in i:
        if not math.isnan(j) and (mi is None or j < mi):
            mi = j
        if not math.isnan(j) and (ma is None or j > ma):
            ma = j
print(f"{mi} - {ma}")

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2, "100m")
data = mapData(aggmap["100m"], mapBestTp)
imshow = ax2.imshow(data, cmap="RdBu", aspect="auto", vmin=-1.75, vmax=1.75)
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")
# ax2.set_ylim(bottom=17)
cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, ticks=[-1, 0, 1])
cbar.ax.set_yticklabels(["$-50$\%", "$\pm$0\%", "+100\%"], rotation=90, va="center", ha="left")  # set ticks of your format
# cbar.ax.set_yticklabels([0 for i in cbar.get_ticks()]) # set ticks of your format
# cbar.ax.set_yticklabels([str(round(i*100))+'\%' for i in cbar.get_ticks()]) # set ticks of your format
drawSeparatorLine(ax2, "100m", "gray")
ax2.set_title("100\,M Elements")
cbar.set_label('Speedup [\%]')
mi = None
ma = None
for i in data:
    for j in i:
        if not math.isnan(j) and (mi is None or j < mi):
            mi = j
        if not math.isnan(j) and (ma is None or j > ma):
            ma = j
print(f"{mi} - {ma}")

savefig("./pdf/evaluation/partition", 0.03)


def mapBestTp(ps):
    tpPart = robustMax([p["throughput"] for p in ps if "AVX" in p["name"]])
    tpNonPart = robustMax([p["throughput"] for p in ps if "Scalar" in p["name"]])
    if tpPart and tpNonPart:
        return tpPart / tpNonPart - 1 if tpPart > tpNonPart else -tpNonPart / tpPart + 1
    else:
        return float("nan")


latexify(cm2inch(8.5), cm2inch(4), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1, "1m")
data = mapData(aggmap["1m"], mapBestTp)
ax1.imshow(data, cmap="YlOrBr", aspect="auto", vmin=0, vmax=1)
drawSeparatorLine(ax1, "1m", "gray")
ax1.set_aspect("equal")
# ax1.set_ylim(bottom=17)
ax1.set_title("1\,M Elements")

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2, "100m")
data = mapData(aggmap["100m"], mapBestTp)
imshow = ax2.imshow(data, cmap="YlOrBr", aspect="auto", vmin=0, vmax=1)
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")
# ax2.set_ylim(bottom=17)
cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, ticks=[0, 0.5, 1])
# cbar.ax.set_yticklabels(['0\%', '+50\%', '+100\%'], rotation=90, va="center")  # vertically oriented colorbar
# cbar.ax.set_yticklabels([str(round(i*100))+'\%' for i in cbar.get_ticks()]) # set ticks of your format
cbar.ax.set_yticklabels(["0\%", "+50\%", "+100\%"], rotation=90, va="center", ha="left")  # set ticks of your format
drawSeparatorLine(ax2, "100m", "gray")
ax2.set_title("100\,M Elements")
cbar.set_label('Speedup [\%]')
savefig("./pdf/evaluation/vectorization", 0.03)
