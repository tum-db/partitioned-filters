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


def analyzeCount(data):
    for name_orig in data.keys():
        name = name_orig.replace('\\', '').replace("Fuse", "Xor_Fuse")
        print(f'  - {name}')
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

            if n_elements_build not in b_count_map:
                b_count_map[n_elements_build] = []

            if f == 0 and not (benchmark['error_occurred'] == True):
                b_count_map[n_elements_build].append(
                    {'name': name, 'k': k, 'size': size, 'time': time, 'fpr': fpr, 'dtlb': dtlb, 'l1d': l1d, 'llc': llc, 'p': p, 'throughput': throughput, 's': benchmark['s'],
                     'bits': benchmark['bits']})


benchmarks = {"filters": ["bloom_blocked", "bloom_naive", "bloom_grouped", "bloom_sectorized", "bloom_sectorized_vertical", "xor", "cuckoo", "morton", "fuse"]}
# benchmarks = {"filters":["fuse"]}
base_path = '../benchmark/paper/perfopt/'
b_count_map = {}

for name in benchmarks["filters"]:
    filename = f'{base_path}{name}.csv'
    print(filename)
    analyzeCount(read_benchmark(filename))

partmap = {}
nonpartmap = {}

for size in b_count_map:
    partmap[size] = {}
    nonpartmap[size] = {}
    for t_exp in np.arange(1., 7.75, 0.25):
        time = math.pow(10, t_exp)
        for b in b_count_map[size]:
            if round(b['s'] * b['k']) <= 26 and b['bits'] < round(b['s'] * b['k']) + 0.1:
                if not time in partmap[size]:
                    partmap[size][time] = b
                elif 10 * b['time'] + time * b['fpr'] < 10 * partmap[size][time]['time'] + time * partmap[size][time]['fpr']:
                    partmap[size][time] = b

                if b['p'] == 0 and not time in nonpartmap[size]:
                    nonpartmap[size][time] = b
                elif b['p'] == 0 and 10 * b['time'] + time * b['fpr'] < 10 * nonpartmap[size][time]['time'] + time * nonpartmap[size][time]['fpr']:
                    nonpartmap[size][time] = b

import matplotlib.ticker as ticker
from matplotlib.colors import ListedColormap
from matplotlib import cm
import math
import copy
from matplotlib.path import Path
import matplotlib.patches as patches
from matplotlib.lines import Line2D


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


def preparePlot(ax):
    def ylogPrintFormat(x, pos):
        return "$10^{{{}}}$".format(4 + round(x / 4))

    ax.yaxis.set_major_formatter(FuncFormatter(ylogPrintFormat))
    ax.yaxis.set_major_locator(ticker.MultipleLocator(4))
    ax.set_ylabel("Number of Keys $n$")
    minorSteps = [4 * (1 - math.log10(x)) for x in range(2, 10)]
    steps = [4 * y + x for x in minorSteps for y in range(4)]
    ax.yaxis.set_minor_locator(ticker.FixedLocator(steps))
    ax.set_ylim(top=16.5, bottom=-0.5)

    def xlogPrintFormat(x, pos):
        return "$10^{{{}}}$".format(1 + round(x / 4))

    ax.xaxis.set_major_formatter(FuncFormatter(xlogPrintFormat))
    ax.xaxis.set_major_locator(ticker.MultipleLocator(8))
    ax.set_xlabel("Work time $t_w$ [ns]")
    ax.set_xlim(left=-0.5, right=26.5)


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
        ax.scatter(best[f][0], best[f][1], color='w', marker=info[f][0], s=1, linewidths=0.3)
    #        if len(legend_elements):
    #            ax.legend(handles=legend_elements, loc='lower left', frameon=False, borderaxespad=0)
    ax.set_title(size)


def plotSome(filename):
    latexify(cm2inch(16.5), cm2inch(4), 2)
    fig = plt.figure()
    num = 131
    for size in aggmap:
        tps = mapData(aggmap[size], mapperFun)
        names = mapData(aggmap[size], bestName)

        ax = fig.add_subplot(num)
        cbar = plt.colorbar(plotFPR(ax, tps, size), format=billions, shrink=0.7)
        cbar.set_label('Throughput [Keys/s]', rotation=90)
        drawScatterSymbols(ax, names, info)
        ax.set_title({"10k": "10\,K", "1m": "1\,M", "100m": "100\,M"}[size] + " Elements")
        drawSeparatorLine(ax, size)
        # if size == "1m":
        #    ax.add_patch(Rectangle((19.5,2.5), 1, 13, linewidth=1, edgecolor='k', facecolor='none', zorder=10, clip_on=False))
        #    ax.add_patch(Rectangle((9.5, 6.5), 16, 1, linewidth=1, edgecolor='k', facecolor='none', zorder=10, clip_on=False))
        num += 1
    plt.tight_layout(pad=1.02)
    savefig("./pdf/{}".format(filename))


def genericMapper(vs):
    fprs = robustMax([v["fpr"] for v in vs])
    sizes = robustMin([v["size"] for v in vs])
    out = [fprs, sizes]
    for name in ["Bloom", "Morton", "Cuckoo", "Xor"]:
        out.append(robustMax([v["throughput"] for v in vs if name in v["name"]]))
    return out


def mapData(data, mapperFun):
    return [[mapperFun(data[size][tw], size, tw) for tw in data[size]] for size in data]


def mapName(ps, size, tw):
    tp = 0
    name = ps["name"][0]
    mapping = {"B": 0, "M": 2, "C": 1, "X": 3}
    return mapping[name]


def mapK(ps, size, tw):
    return ps["k"]


def mapP(ps, size, tw):
    return ps["p"]


def mapS(ps, size, tw):
    return round(ps["s"] * 100) / 100


def mapBits(ps, size, tw):
    return round(ps["bits"] * 10) / 10


def mapTp(ps, size, tw):
    return ps["throughput"]


def mapFPR(ps, size, tw):
    return ps["fpr"]


def mapSpeedup(ps, size, tw):
    return round(partmap[size][tw]["throughput"] / nonpartmap[size][tw]["throughput"] * 100) / 100 - 1.0


def mapVariantName(ps, size, tw):
    if "Bloom" in ps["name"]:
        if "Blocked512" in ps["name"]:
            return "5"
        else:
            return ps["name"][6]
    elif "Fuse" in ps["name"]:
        return "F"
    elif "Xor" in ps["name"]:
        return "X"
    elif "Morton" in ps["name"]:
        return "M"
    elif "Cuckoo" in ps["name"]:
        return "C"
    else:
        return " "


from matplotlib.patches import Patch, Rectangle


def drawSeparatorLine(ax, color="w"):
    verts = [[11.5, -0.5], [11.5, 6.5], [14.5, 6.5], [14.5, 7.5], [15.5, 7.5], [15.5, 11.5], [16.5, 11.5], [16.5, 12.5], [17.5, 12.5], [17.5, 13.5], [18.5, 13.5], [18.5, 16.5]]
    codes = [Path.MOVETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO]
    path = Path(verts, codes)
    patch = patches.PathPatch(path, facecolor='none', edgecolor=color, lw=.75)
    ax.add_patch(patch)


info = ["Bloom", "Cuckoo", "Morton", "Xor"]

latexify(cm2inch(8.5), cm2inch(3.05), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=2, nrows=1, width_ratios=[1, 1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1)
names = mapData(nonpartmap, mapName)
newcmp = ListedColormap([colors[x.lower()] for x in info])
ax1.imshow(names, cmap=newcmp, aspect="auto")
ax1.set_aspect("equal")
legend_elements = []
for f in info:
    if f != 'Morton':
        legend_elements.append(Patch(facecolor=colors[f.lower()], label=f))
allfig = plt.gcf()
# allfig.legend(handles=legend_elements, loc='lower left', handlelength=1, handleheight=1, frameon=False, borderaxespad=0)
allfig.legend(handles=legend_elements, ncol=4, bbox_to_anchor=(0.25, 1.01), loc='upper center', borderaxespad=0, frameon=False, columnspacing=1)
drawSeparatorLine(ax1)

ax2 = fig.add_subplot(spec[0, 1])
format_axes_imshow(ax2)
preparePlot(ax2)
data = mapData(nonpartmap, mapFPR)
mi = None
ma = None

for i in data:
    for j in i:
        if mi is None or j < mi:
            mi = j
        if ma is None or j > ma:
            ma = j

print(f"{mi} {ma}")
imshow = ax2.imshow(data, cmap="cividis", aspect="auto", norm=LogNorm(vmin=1e-7, vmax=0.2))
ax2.set_ylabel("")
ax2.set_yticklabels([])
ax2.set_aspect("equal")


def specialFormat(x, _):
    if x == 1: return "$1$"
    if x in [1, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8]:
        return "$10^{{\\scalebox{{0.75}}[1.0]{{-}}{}}}$".format(round(abs(math.log10(x))))


cbar = plt.colorbar(imshow, aspect=40, pad=0, shrink=0.7, ticks=[1, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7], format=ticker.FuncFormatter(specialFormat))

cbar.ax.yaxis.set_minor_formatter(ticker.FuncFormatter(specialFormat))
cbar.set_label('False-positive rate $\\varepsilon$')
drawSeparatorLine(ax2)

savefig("./pdf/perfopt/skyline", 0.03)

from matplotlib.patches import Patch, Rectangle


def drawSeparatorLine(ax, color="w"):
    verts = [[11.5, -0.5], [11.5, 6.5], [14.5, 6.5], [14.5, 7.5], [15.5, 7.5], [15.5, 10.5], [16.5, 10.5], [16.5, 12.5], [17.5, 12.5], [17.5, 13.5], [18.5, 13.5], [18.5, 16.5]]
    codes = [Path.MOVETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.LINETO]
    path = Path(verts, codes)
    patch = patches.PathPatch(path, facecolor='none', edgecolor=color, lw=.75)
    ax.add_patch(patch)


info = ["Bloom", "Cuckoo", "Morton", "Xor"]

latexify(cm2inch(4), cm2inch(3.05), 2)
fig = plt.figure(constrained_layout=True)
spec = fig.add_gridspec(ncols=1, nrows=1, width_ratios=[1])

ax1 = fig.add_subplot(spec[0, 0])
format_axes_imshow(ax1)
preparePlot(ax1)
names = mapData(partmap, mapName)
newcmp = ListedColormap([colors[x.lower()] for x in info])
ax1.imshow(names, cmap=newcmp, aspect="auto")
ax1.set_aspect("equal")
drawSeparatorLine(ax1)

legend_elements = []
for f in info:
    if f != "Morton":
        legend_elements.append(Patch(facecolor=colors[f.lower()], label=f))
allfig = plt.gcf()
allfig.legend(handles=legend_elements, ncol=4, bbox_to_anchor=(0.5, 1.02), loc='upper center', borderaxespad=0, frameon=False, columnspacing=1)

savefig("./pdf/perfopt/skyline_part", 0.01)
