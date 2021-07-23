def blend(color, factor):
    return int(255 - (255 - color) * factor)


def rgb(red, green, blue, factor=1):
    """Return color as #rrggbb for the given color values."""
    return '#%02x%02x%02x' % (blend(red, factor), blend(green, factor), blend(blue, factor))


colors = {
    'blue': rgb(55, 126, 184),
    'lightblue': rgb(55, 126, 184, 0.6),
    'oldgreen': rgb(77, 175, 74),
    'oldlightgreen': rgb(77, 175, 74, 0.6),
    'orange': rgb(255, 127, 0),
    'lightorange': rgb(255, 127, 0, 0.6),
    'red': rgb(228, 26, 28),
    'lightred': rgb(228, 26, 28, 0.75),
    'black': rgb(0, 0, 0),
    'morton': "#FFD200",
    'cuckoo': "#FF7F00",
    'lightmorton': rgb(255, 210, 0, 0.6),
    'lightcuckoo': rgb(255, 127, 0, 0.6),
    'xor': "#CD161C",
    'bloom': "#23507A",
    'lightbloom': rgb(35, 80, 122, 0.6),
    'fuse': "#29021d",
    'lightviolet': "#984EA3",
    'violet': "#67356F",
    'lightgreen': "#4DAF4A",
    'green': "#3C893A",
    'turquoise': "#45E2CD",
    'pink': "#F028F0",
}

from matplotlib.ticker import FuncFormatter


def kilos(x, pos):
    """The two args are the value and tick position"""
    return '%1.0f\\,K' % (x * 1e-3)


def millions(x, pos=None):
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


def speedup(x, pos=None):
    sign = '+' if x > 0 else ''
    return sign + '%.0f' % (x * 100) + '\%'


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

    csv['name'] = split_name.apply(lambda x: "_".join(x[0:(len(x) - 1)]))

    data = {}
    for _, row in csv.iterrows():
        data_row = {}
        for label, item in row.iteritems():
            data_row[label] = item

        if data_row['fixture'] == 'Construct':
            data_row['throughput'] = data_row['n_elements_build'] / (data_row['duration'] / 1000)
        elif data_row['fixture'] == 'Count' or data_row['fixture'] == 'MTCount':
            data_row['throughput'] = data_row['n_elements_lookup'] / (data_row['duration'] / 1000)

        name = row['name']
        if name not in data:
            data[name] = []
        data[name].append(data_row)

    return data


import matplotlib
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, FuncFormatter, FixedLocator
from matplotlib.patches import Rectangle
import numpy as np

matplotlib.use('pgf')


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
              'axes.titlesize': 7,
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
              'text.latex.preamble': r'\usepackage{hyperref} \usepackage{amssymb} \usepackage{ifsym} \usepackage[T1]{fontenc} \usepackage{libertine} \usepackage{graphicx}',
              'pgf.preamble': r'\usepackage{hyperref} \usepackage{amssymb} \usepackage{ifsym} \usepackage[T1]{fontenc} \usepackage{libertine} \usepackage{graphicx}'
              }

    matplotlib.rcParams.update(params)


def logPrintFormat(x, pos):
    if x < 1:
        return "$10^{{\\scalebox{{0.75}}[1.0]{{-}}{}}}$".format(round(-math.log10(x)))
    else:
        return "$10^{}$".format(round(math.log10(x)))


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
        ax.yaxis.set_major_formatter(FuncFormatter(logPrintFormat))
        locmin = matplotlib.ticker.LogLocator(base=10.0, subs=(np.arange(0, 1, 0.1)), numticks=12)
        ax.yaxis.set_minor_locator(locmin)
        ax.yaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())
    else:
        ax.yaxis.set_minor_locator(AutoMinorLocator(n=2))
        ax.yaxis.grid(b=True, which='minor', linestyle=':')

    ax.xaxis.set_ticks_position('bottom')
    ax.xaxis.set_tick_params(direction='out', color=spine_color)

    if xscale == 'log':
        locmaj = matplotlib.ticker.LogLocator(base=10, numticks=12)
        ax.xaxis.set_major_locator(locmaj)
        ax.xaxis.set_major_formatter(FuncFormatter(logPrintFormat))
        locmin = matplotlib.ticker.LogLocator(base=10.0, subs=(np.arange(0, 1, 0.1)), numticks=12)
        ax.xaxis.set_minor_locator(locmin)
        ax.xaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())

    ax.yaxis.set_ticks_position('left')
    ax.yaxis.set_tick_params(direction='out', color=spine_color)
    ax.yaxis.grid(b=True, which='major')

    ax.tick_params(axis='both', which='major', pad=0.5)

    return ax


def format_axins(ax, xscale='linear', yscale='linear'):
    spine_color = 'black'
    for spine in ['left', 'top', 'right', 'bottom']:
        ax.spines[spine].set_color(spine_color)
        ax.spines[spine].set_linewidth(0.5)

    ax.set_xscale(xscale)
    ax.set_yscale(yscale)

    ax.xaxis.set_visible(False)
    # ax.yaxis.set_visible(False)

    if yscale == 'log':
        locmaj = matplotlib.ticker.LogLocator(base=10, numticks=12)
        ax.yaxis.set_major_locator(locmaj)
        ax.yaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())
    else:
        ax.yaxis.grid(b=True, which='minor', linestyle=':')

    ax.yaxis.grid(b=True, which='major')
    ax.set_yticklabels([])
    ax.xaxis.set_minor_formatter(matplotlib.ticker.NullFormatter())

    for tic in ax.yaxis.get_major_ticks():
        tic.tick1line.set_visible(False)
    for tic in ax.yaxis.get_minor_ticks():
        tic.tick1line.set_visible(False)

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


def savefig(path):
    dir = os.path.dirname(path)
    if not os.path.exists(dir):
        os.makedirs(dir)
    plt.savefig(path + ".pgf", bbox_inches='tight', pad_inches=0)
    plt.savefig(path + ".pdf", bbox_inches='tight', pad_inches=0)


import math


def analyzeFPR(data, skyline):
    for name in data.keys():
        b = 0
        cr = 0
        ota = 0

        if 'Cuckoo' in name and not 'Opt' in name:
            b = int(name.replace('Cuckoo', ''))
            skyline_name = 'Cuckoo'
        elif 'Morton' in name and not 'Opt' in name:
            b = int(name.replace('Morton', '').split('_')[0])
            cr = int(name.replace('Morton', '').split('_')[1])
            ota = int(name.replace('Morton', '').split('_')[2])
            skyline_name = f'Morton'
        else:
            b = 0
            skyline_name = name

        if skyline_name not in skyline.keys():
            skyline[skyline_name] = {}

        last_bits = 0

        for benchmark in data[name]:
            k = benchmark['k']
            bits = round(benchmark['bits'] * 4) / 4
            failures = benchmark['failures']
            fpr = benchmark['fpr']
            s = benchmark['s']

            if failures == 100:
                bits = round(k * s * 4) / 4
            if bits == last_bits:
                bits += 0.25
            last_bits = bits

            if failures <= 0 and (bits not in skyline[skyline_name].keys() or skyline[skyline_name][bits]['fpr'] > fpr):
                skyline[skyline_name][bits] = {'k': k, 'fpr': fpr, 's': s, 'b': b, 'cr': cr, 'ota': ota}


def analyzeStacked(data, skyline):
    for name in data.keys():
        if name == 'InitialiseData':
            continue

        optimization = name.split('_', 1)[1]
        skyline_name = name.split('_', 1)[0]

        if skyline_name not in skyline.keys():
            skyline[skyline_name] = {'Construct': {}, 'Count': {}}

        for benchmark in data[name]:
            if benchmark['failures'] <= 0:
                skyline[skyline_name][benchmark['fixture']][optimization] = {'t': benchmark['throughput']}


def analyzeFailures(data, skyline, xaxis):
    for name in data.keys():
        if name not in skyline.keys():
            skyline[name] = {}
        for benchmark in data[name]:
            x = benchmark[xaxis]
            failures = benchmark['failures']
            fpr = benchmark['fpr']
            s = benchmark['s']
            k = benchmark['k']
            if benchmark['bits'] <= s * k * 1.05 and failures == 0 and (x not in skyline[name].keys() or skyline[name][x]['s'] > s):
                skyline[name][x] = {'fpr': fpr, 's': s}


def analyzeHashing(data, skyline):
    for name in data.keys():
        if name not in skyline:
            skyline[name] = {}

        for benchmark in data[name]:
            fixture = benchmark['fixture']
            t = benchmark['duration']
            f = benchmark['failures']
            if f == 0 and not (benchmark['error_occurred'] == True) and fixture not in skyline[name].keys():
                skyline[name][fixture] = {'t': t}

    for name in {'BloomBlocked', 'Cuckoo', 'Xor', 'Morton'}:
        for fixture in skyline[f'{name}Cityhash']:
            city = skyline[f'{name}Cityhash'][fixture]['t']
            for hashfunc in {'Murmur', 'Fasthash', 'Mul'}:
                t = skyline[f'{name}{hashfunc}'][fixture]['t']
                skyline[f'{name}{hashfunc}'][fixture]['speedup'] = -2 if t == 0 or math.isnan(t) else city / t


def analyzeCompetitors(data, skyline):
    for name in data.keys():
        if name not in skyline.keys():
            skyline[name] = {'Construct': {}, 'Count': {}}

        for benchmark in data[name]:
            if benchmark['failures'] <= 0:
                skyline[name][benchmark['fixture']] = {'t': benchmark['throughput']}


def analyzePerKey(data, skyline):
    for name in data.keys():
        if name not in skyline.keys():
            skyline[name] = {'Construct': {}, 'Count': {}}
        size1 = 0
        for benchmark in data[name]:
            fixture = benchmark['fixture']
            n_elements_build = benchmark['n_elements_build']
            n_elements_lookup = benchmark['n_elements_lookup']
            n_elements = n_elements_build if fixture == 'Construct' else n_elements_lookup
            size = benchmark['size']
            f = benchmark['failures']
            t = benchmark['duration'] * 1e6 / n_elements
            throughput = n_elements * 1e3 / benchmark['duration']
            dtlb = benchmark['DTLB-misses'] / n_elements
            l1d = benchmark['L1D-misses'] / n_elements
            llc = benchmark['LLC-misses'] / n_elements
            p = math.log(benchmark['n_partitions'], 2)
            if p == 0:
                size1 = size
            if f == 0 and size / size1 < 1.05 and not (benchmark['error_occurred'] == True) and (
                    n_elements_build not in skyline[name][fixture].keys() or skyline[name][fixture][n_elements_build]['t'] > t):
                skyline[name][fixture][n_elements_build] = {'size': size, 't': t, 'dtlb': dtlb, 'l1d': l1d, 'llc': llc, 'p': p, 'throughput': throughput}


def analyzeCorridor(data, skyline):
    for name in data.keys():
        if name == "InitialiseData":
            continue
        if name not in skyline:
            skyline[name] = {'Count': {}, 'Construct': {}}
        for benchmark in data[name]:
            n_elements_build = benchmark['n_elements_build']
            fixture = benchmark['fixture']
            size = benchmark['size']
            f = benchmark['failures']
            t = benchmark['duration']
            p = math.log(benchmark['n_partitions'], 2)
            if f == 0 and not (benchmark['error_occurred'] == True):
                if not n_elements_build in skyline[name][fixture]:
                    skyline[name][fixture][n_elements_build] = {}
                skyline[name][fixture][n_elements_build][p] = {'size': size, 't': t, 'p': p}


def analyzeSIMD(data, skyline):
    for name in data.keys():
        size1 = 0
        skyline_name = name.replace('Partitioned', '')
        if skyline_name not in skyline:
            skyline[skyline_name] = {'Count': {}}
        for benchmark in data[name]:
            n_elements_build = benchmark['n_elements_build']
            size = benchmark['size']
            f = benchmark['failures']
            t = benchmark['duration']
            p = math.log(benchmark['n_partitions'], 2)
            e = 'Partitioned' in name
            if p == 0:
                size1 = size
            if f == 0 and size / size1 < 1.05 and not (benchmark['error_occurred'] == True) and (
                    n_elements_build not in skyline[skyline_name]['Count'].keys() or skyline[skyline_name]['Count'][n_elements_build]['t'] > t):
                skyline[skyline_name]['Count'][n_elements_build] = {'size': size, 't': t, 'p': p, 'e': e}


def analyzeMultiThreading(prefix, data, skyline, numa=False, partitions=None):
    for name in data.keys():
        size1 = 0
        skyline_name = f'{prefix}_{name}'
        if skyline_name not in skyline:
            skyline[skyline_name] = {}
        for benchmark in data[name]:
            n_elements_build = benchmark['n_elements_build']
            size = benchmark['size']
            f = benchmark['failures']
            t = benchmark['duration']
            p = math.log(benchmark['n_partitions'], 2)
            n_threads = benchmark['n_threads']
            e = 'Partitioned' in name
            if e and partitions is not None and benchmark['n_partitions'] != partitions:
                continue
            if n_elements_build == 100000000 and f == 0 and not (benchmark['error_occurred'] == True) and (n_threads not in skyline[skyline_name].keys() or skyline[skyline_name][n_threads]['t'] > t):
                skyline[skyline_name][n_threads] = {'size': size, 't': t, 'p': p, 'e': e}


def plotFPR(config, skyline, ax, yaxis):
    ax.set_xlabel('Bits per key ($m/n$)')
    ax.set_xlim(4.75, 25.25)
    handles = []
    for name in config.keys():
        x = []
        y = []
        for bits in sorted(list(skyline[name])):
            if 5 <= bits <= 25:
                x.append(bits)
                y.append(skyline[name][bits][yaxis])
        handles.append(ax.plot(x, y, label=config[name]['label'], color=config[name]['color'], linestyle=config[name]['linestyle'], linewidth=config[name]['linewidth'])[0])
    return handles


def plotFailure(config, skyline, ax, k=True):
    ax.set_xlabel("Minimal space overhead $s$")
    handles = []
    for name in config.keys():
        x = []
        y = []
        for i in sorted(list(skyline[name])):
            if (i and 1 <= i <= 32) or (not k and i >= config[name]['min']):
                x.append(i)
                y.append(skyline[name][i]['s'])
        handles.append(ax.plot(x, y, label=config[name]['label'], color=config[name]['color'], linewidth=1)[0])
    return handles


def plotStacked(config, opt_config, skyline, ax):
    width = 0.4
    offset = 0.02
    p = []
    for i, name in enumerate(config.keys()):
        for (fixture, factor) in [('Construct', -1), ('Count', 1)]:
            bottom = 0
            for opt in config[name][fixture]:
                last_opt = opt.split('_')[-1]
                throughput = skyline[name][fixture][opt]['t']
                b = ax.bar([i + (width + offset) / 2 * factor], [throughput - bottom], width, bottom=bottom, color=opt_config[last_opt]['color'], zorder=10, edgecolor='gray', linewidth=0)
                if fixture == 'Count' and i == len(config.keys()) - 1:
                    p.append(b)
                bottom = throughput
            ax.text(b[0].get_x() + width / 2, -3e6, 'Lookup' if fixture == 'Count' else 'Build', ha='center', va='top', color='k', fontsize=5, zorder=20)
        props = dict(facecolor='white', boxstyle='square,pad=0.2', alpha=1, lw=0.5)
        print(bottom)
        # ax.text(b[0].get_x(), 190e6, f"FPR: {config[name]['fpr']}\nSize: {config[name]['size']}", ha='center', va='bottom', bbox=props, color='k', fontsize=5, zorder=20)
        ax.text(b[0].get_x(), bottom + 10e6, f"FPR: {config[name]['fpr']}\nSize: {config[name]['size']}", ha='center', va='bottom', bbox=props, color='k', fontsize=5, zorder=20)

    ax.axhline(y=0, color='k', linestyle='-', lw=1, zorder=20)
    ax.yaxis.set_major_formatter(mills)

    allfig = plt.gcf()
    label0 = 'Baseline (\\hyperref[s:addr]{Section 4.1} \\& \\hyperref[s:hash]{Section 4.2})'
    label1 = 'Partitioning (\\hyperref[s:part]{Section 4.3})'
    label2 = 'Vectorization (\\hyperref[s:vec]{Section 4.4})'
    # allfig.legend((p[0]), ('label0'), bbox_to_anchor=(0, 1), loc='upper left', borderaxespad=0, frameon=False)
    allfig.legend((p[0],), (label0,), ncol=2, bbox_to_anchor=(0.1, 1.075), loc='upper left', borderaxespad=0, frameon=False, columnspacing=1)
    allfig.legend((p[1], p[2]), (label1, label2), ncol=2, bbox_to_anchor=(0.1, 1), loc='upper left', borderaxespad=0, frameon=False, columnspacing=1)


def plotHashing(config, skyline, ax):
    handles = {}
    width = 0.175
    offset = 0.025
    for i, name in enumerate(["BloomBlocked", "Cuckoo", "Morton", "Xor"]):
        for (fixture, factor, color) in [('Construct', -1, 'color1'), ('Count', 1, 'color1')]:
            for j, hash_func in enumerate(config.keys()):
                speedup = skyline[f'{name}{hash_func}'][fixture]['speedup']
                handles[hash_func] = ax.bar([i + factor * (width + offset) + width / 2 * config[hash_func]['factor']], [speedup - 1], width, 0, color=config[hash_func][color], zorder=10)
            ax.text(i + factor * (width + offset), -0.02, 'Lookup' if fixture == 'Count' else 'Build', ha='center', va='top', color='k', fontsize=5, zorder=20)

    return handles


def plotCompetitors(config, fixture, skyline, ax, legend=True):
    width = 0.35
    ind = []
    label = []
    p = {}
    for i, name in enumerate(config.keys()):
        ind.append(i)
        label.append(config[name]['label'])
        t_competitor = skyline[name][fixture]['t']
        t_our = skyline[config[name]['competitor']][fixture]['t']
        p['Competitor'] = ax.bar([i - width / 2], [t_competitor], width, color=colors['blue'], zorder=10)
        p['Ours'] = ax.bar([i + width / 2], [t_our], width, color=colors['orange'], zorder=10)

        bar0 = p['Competitor'][0]
        bar1 = p['Ours'][0]
        posText = (bar0.get_height() + bar1.get_height()) / 2
        if t_competitor <= t_our:
            middle = bar0.get_x() + bar0.get_width() / 2
        else:
            middle = bar1.get_x() + bar1.get_width() / 2
        height = max(bar0.get_height(), bar1.get_height())
        ax.plot([bar0.get_x(), bar0.get_x() + bar0.get_width() * 2], [height, height], 'k-', lw=0.5, zorder=20)
        ax.plot([middle, middle], [bar0.get_height(), bar1.get_height()], 'k-', lw=0.5, zorder=20)
        ax.text(bar1.get_x(), height + 0.005e9, speedup(t_our / t_competitor - 1), ha='center', va='bottom', color='k')
        ax.text(bar1.get_x() + width / 2, -1e6, 'Lookup' if fixture == 'Count' else 'Build', ha='center', va='top', color='k', fontsize=5, zorder=20)

    l = ax.legend((p['Competitor'], p['Ours']), ('Competitor', 'Ours'), labelspacing=0, ncol=2, bbox_to_anchor=(0.99, 1), borderaxespad=0, framealpha=1, edgecolor='black', fancybox=False)
    l.set_visible(legend)
    l.get_frame().set_linewidth(0.5)
    ax.set_xticks(ind)
    ax.set_xticklabels(label, rotation=45, ha='right', rotation_mode="anchor")
    ax.yaxis.set_major_formatter(gigs)
    ax.set_ylim(0, 0.41e9)
    ax.axhline(y=0, color='k', linestyle='-', lw=1, zorder=20)


def plotFilterSize(config, fixture, skyline, ax, yaxis, min_size, max_size, y_caches, TLB=False):
    ax.set_xlabel('Filter size $m$ [KiB]')

    if 'speedup' in yaxis:
        ax.axhline(y=0, color='k', linestyle='-', lw=1)
    caches = {'L1': 32, 'L2': 1024, 'L3': 1024 * 19.25} if not TLB else {'dTLB': 256, 'L2 TLB': 6144}
    for name in caches:
        if min_size < caches[name] < max_size:
            ax.axvline(x=caches[name], color='k', linestyle='-', lw=0.75)
            props = dict(boxstyle='round', facecolor='white', alpha=1, lw=0.75)
            ax.text(caches[name], y_caches, name, horizontalalignment='center', bbox=props, fontsize=6)
    handles = []
    for name in config.keys():
        x = []
        y = []
        for n_elements in sorted(list(skyline[name][fixture])):
            size = skyline[name][fixture][n_elements]['size'] / 1024
            if (min_size < size < max_size):
                x.append(size)
                val = skyline[name][fixture][n_elements][yaxis]
                if math.isnan(val):
                    val = -2
                y.append(val)
        handles.append(ax.plot(x, y, label=config[name]['label'], color=config[name]['color'], linestyle=config[name]['linestyle'], linewidth=config[name]['linewidth'], clip_on=False))
    return handles


def plotCorridor(config, skyline, ax, fixture, min_size, max_size, y_caches):
    ax.set_xlabel("Filter size in [KiB]")
    caches = {'L1': 32, 'L2': 1024, 'L3': 1024 * 19.25}
    for name in caches:
        if min_size < caches[name] < max_size:
            ax.axvline(x=caches[name], color='k', linestyle='-', lw=0.75)
            props = dict(boxstyle='round', facecolor='white', alpha=1, lw=0.75)
            ax.text(caches[name], y_caches, name, horizontalalignment='center', bbox=props, fontsize=6)

    handles = []
    for name in config.keys():
        x = []
        y = []
        y1 = []
        y2 = []
        y15 = []
        y25 = []
        for n_elements in sorted(list(skyline[name][fixture])):
            size = skyline[name][fixture][n_elements]['size'] / 1024
            if min_size < size < max_size:
                x.append(skyline[name][fixture][n_elements]['size'] / 1024)
                y.append(skyline[name][fixture][n_elements]['min'])
                y1.append(min(skyline[name][fixture][n_elements]['corridor']))
                y2.append(max(skyline[name][fixture][n_elements]['corridor']))
                y15.append(min(skyline[name][fixture][n_elements]['corridor5']))
                y25.append(max(skyline[name][fixture][n_elements]['corridor5']))
        # ax.fill_between(x, y1, y2, color=config[name]['color'], linestyle=config[name]['linestyle'], linewidth=config[name]['linewidth'], alpha=0.25)
        ax.fill_between(x, y15, y25, color=config[name]['color'], linestyle=config[name]['linestyle'], linewidth=config[name]['linewidth'], alpha=0.25)
        handles.append(ax.plot(x, y, label=config[name]['label'], color=config[name]['color'], linestyle=config[name]['linestyle'], linewidth=config[name]['linewidth'])[0])


def plotScaleup(config, skyline, ax, base_name, datapoints):
    for postfix in config.keys():
        name = f'{base_name}{postfix}'
        x = []
        y = []
        lbls = [str(d) for d in datapoints]
        for n_threads in datapoints:
            val = skyline[name][n_threads]['scaleup']
            if math.isnan(val):
                val = 0
            y.append(val)
        ax.plot(lbls, y, label=f'{config[postfix]["label"]}', color=config[postfix]['color'], linestyle='solid', linewidth=config[postfix]['linewidth'])


skylineStacked = {}

analyzeStacked(read_benchmark('../benchmark/paper/introduction/bloom_blocked_stacked_construct.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/bloom_blocked_stacked_count.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/bloom_sectorized_stacked_construct.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/bloom_sectorized_stacked_count.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/cuckoo_stacked_construct.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/cuckoo_stacked_count.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/morton_stacked_construct.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/morton_stacked_count.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/xor_stacked_construct.csv'), skylineStacked)
analyzeStacked(read_benchmark('../benchmark/paper/introduction/xor_stacked_count.csv'), skylineStacked)

config = {
    'BloomBlocked': {'label': 'Bloom',
                     'Construct': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned'],
                     'Count': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned', 'Lemire_Murmur_Partitioned_SIMD'],
                     'fpr': '0.41\%', 'size': '143 MiB'},
    'Cuckoo': {'label': 'Cuckoo',
               'Construct': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned'],
               'Count': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned', 'Lemire_Murmur_Partitioned_SIMD'],
               'fpr': '2.93\%', 'size': '101 MiB'},
    'Morton': {'label': 'Cuckoo',
               'Construct': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned'],
               'Count': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned', 'Lemire_Murmur_Partitioned_SIMD'],
               'fpr': '0.42\%', 'size': '132 MiB'},
    'Xor': {'label': 'Xor',
            'Construct': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned'],
            'Count': ['Lemire_Murmur', 'Lemire_Murmur_Partitioned', 'Lemire_Murmur_Partitioned_SIMD'],
            'fpr': '0.39\%', 'size': '117 MiB'},
}
lightred = rgb(228, 26, 28, 0.8)
opt_config = {
    'Baseline': {'label': 'Baseline', 'color': lightred},
    'Lemire': {'label': 'Baseline', 'color': lightred},
    'Murmur': {'label': 'Baseline', 'color': lightred},
    'Mul': {'label': 'Baseline', 'color': lightred},
    'SIMD': {'label': 'Vectorization', 'color': colors['lightorange']},
    'Partitioning': {'label': 'Partitioning', 'color': colors['lightgreen']},
    'Partitioned': {'label': 'Partitioning', 'color': colors['lightgreen']},
}

latexify(cm2inch(8.5), cm2inch(3.8), 2)

fig = plt.figure()
ax = fig.add_subplot(111)
format_axes(ax)
plotStacked(config, opt_config, skylineStacked, ax)
# ax.legend().set_visible(False)
ax.set_xticks([0, 1, 2, 3])
ax.set_xticklabels([
    "\\textbf{Bloom}\n(\\hyperref[s:bloom]{Section 3.1})",
    "\\textbf{Cuckoo}\n(\\hyperref[s:cuckoo]{Section 3.2.1})",
    "\\textbf{Morton}\n(\\hyperref[s:morton]{Section 3.2.2})",
    "\\textbf{Xor}\n(\\hyperref[s:xor]{Section 3.2.3})"
])
ax.tick_params(axis='x', which='major', pad=2.5)
ax.set_ylabel("Throughput [Keys/s]")
ax.set_ylim(0, 215e6)
savefig("./pdf/introduction/stacked")

skylineFPR = {}

analyzeFPR(read_benchmark('../benchmark/paper/background/bloom/bloom_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/cuckoo/fingerprintbased_cuckoo_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/cuckoo_opt/fingerprintbased_cuckoo_opt_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/morton_opt/fingerprintbased_morton_opt_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/morton2/fingerprintbased_morton2_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/morton3/fingerprintbased_morton3_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/morton7/fingerprintbased_morton7_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/xor/fingerprintbased_xor_fpr.csv'), skylineFPR)
analyzeFPR(read_benchmark('../benchmark/paper/background/fingerprintbased/fuse/fingerprintbased_fuse_fpr.csv'), skylineFPR)

skylineFailureElements = {}

analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/xor/fingerprintbased_xor_failure_elements.csv'), skylineFailureElements, 'n_elements_build')
analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/fuse/fingerprintbased_fuse_failure_elements.csv'), skylineFailureElements, 'n_elements_build')
analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/morton_opt/fingerprintbased_morton_opt_failure_elements.csv'), skylineFailureElements, 'n_elements_build')
analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/cuckoo_opt/fingerprintbased_cuckoo_opt_failure_elements.csv'), skylineFailureElements, 'n_elements_build')

skylineFailureK = {}

analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/xor/fingerprintbased_xor_failure_k.csv'), skylineFailureK, 'k')
analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/fuse/fingerprintbased_fuse_failure_k.csv'), skylineFailureK, 'k')
analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/morton_opt/fingerprintbased_morton_opt_failure_k.csv'), skylineFailureK, 'k')
analyzeFailures(read_benchmark('../benchmark/paper/background/fingerprintbased/cuckoo_opt/fingerprintbased_cuckoo_opt_failure_k.csv'), skylineFailureK, 'k')

configBloom = {
    'BloomSectorized256': {'label': 'Sectorized', 'color': colors['lightred'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'BloomBlocked64': {'label': 'Register-blocked', 'color': colors['lightorange'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'BloomCacheSectorized2': {'label': 'Cache-sectorized', 'color': colors['orange'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'BloomStandard': {'label': 'Na\\"ive', 'color': colors['blue'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'BloomBlocked512': {'label': 'Cache-blocked', 'color': colors['bloom'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
}

configFingerprintFPR = {
    'Morton': {'label': '', 'color': colors['lightmorton'], 'marker': markers['circle'], 'linestyle': 'dashed', 'linewidth': 0.75},
    'Cuckoo': {'label': '', 'color': colors['lightcuckoo'], 'marker': markers['circle'], 'linestyle': 'dashed', 'linewidth': 0.75},
    'MortonOpt': {'label': 'Morton', 'color': colors['morton'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'CuckooOpt': {'label': 'Cuckoo', 'color': colors['cuckoo'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'Xor': {'label': 'Xor', 'color': colors['xor'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
    'Fuse': {'label': 'Xor (Fuse)', 'color': colors['fuse'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1},
}

configFingerprintK = {
    'MortonOpt': {'label': 'Morton filter', 'color': colors['morton'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1, 'min': 1000},
    'CuckooOpt': {'label': 'Cuckoo filter', 'color': colors['cuckoo'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1, 'min': 100},
    'Xor': {'label': 'Xor filter', 'color': colors['xor'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1, 'min': 100},
    'Fuse': {'label': 'Xor filter (Fuse)', 'color': colors['fuse'], 'marker': markers['circle'], 'linestyle': 'solid', 'linewidth': 1, 'min': 100},
}

from mpl_toolkits.axes_grid1.inset_locator import zoomed_inset_axes, inset_axes
from mpl_toolkits.axes_grid1.inset_locator import mark_inset
from mpl_toolkits.axes_grid1.inset_locator import TransformedBbox, BboxPatch, BboxConnector

latexify(cm2inch(9), cm2inch(3.5), 2)


def plot_inset(parent_axes, inset_axes, loc1a=1, loc1b=1, loc2a=2, loc2b=2, **kwargs):
    rect = TransformedBbox(inset_axes.viewLim, parent_axes.transData)

    pp = BboxPatch(rect, fill=False, color='w', ec='gray', lw=0.5, zorder=2, ls='--')
    parent_axes.add_patch(pp)

    p1 = BboxConnector(inset_axes.bbox, rect, loc1=loc1a, loc2=loc1b, ec='gray', lw=0.5, zorder=2, ls='--')
    parent_axes.add_patch(p1)
    p1.set_clip_on(False)
    p2 = BboxConnector(inset_axes.bbox, rect, loc1=loc2a, loc2=loc2b, ec='gray', lw=0.5, zorder=2, ls='--')
    parent_axes.add_patch(p2)
    p2.set_clip_on(False)

    return pp, p1, p2


if True:
    fig = plt.figure()
    ax0 = fig.add_subplot(121)
    handles = plotFPR(configBloom, skylineFPR, ax0, 'fpr')
    ax0.set_ylabel("False-positive rate $\\varepsilon$")
    format_axes(ax0, 'linear', 'log')
    ax0.set_xticks([5, 10, 15, 20, 25])

    axins = zoomed_inset_axes(ax0, 2, loc='lower left')
    # axins = inset_axes(ax0, 0.25, 0.25, loc='lower left', bbox_to_anchor=(0, 0)) # no zoom
    plotFPR(configBloom, skylineFPR, axins, 'fpr')
    axins.set_xlim(6, 10.5)
    axins.set_ylim(0.009, 0.065)
    format_axins(axins, 'linear', 'log')
    patch, pp1, pp2 = plot_inset(ax0, axins, loc1a=2, loc1b=3, loc2a=1, loc2b=4, fc="none", ec="0.5", lw=0.2)

    handles = [handles[3], handles[4], handles[0], handles[2], handles[1]]
    allfig = plt.gcf()
    allfig.legend(handles=[handles[0], handles[2], handles[1], handles[4], handles[3]], bbox_to_anchor=(0.5, 1.1), loc='upper center', ncol=3, borderaxespad=0, frameon=False)

    ax1 = fig.add_subplot(122)
    plotFPR(configBloom, skylineFPR, ax1, 'k')
    ax1.set_ylabel("Optimal $k$")
    format_axes(ax1)
    ax1.set_xticks([5, 10, 15, 20, 25])
    plt.tight_layout()
    savefig("./pdf/background/bloom_fpr")

if True:
    fig = plt.figure()
    ax0 = fig.add_subplot(121)
    plotFPR(configFingerprintFPR, skylineFPR, ax0, 'fpr')
    ax0.legend().set_visible(False)
    ax0.set_ylabel("False-positive rate $\\varepsilon$")
    format_axes(ax0, 'linear', 'log')
    ax0.set_xticks([5, 10, 15, 20, 25])

    axins = zoomed_inset_axes(ax0, 2, loc='lower left')
    # axins = inset_axes(ax0, 0.25, 0.25, loc='lower left', bbox_to_anchor=(0, 0)) # no zoom
    plotFPR(configFingerprintFPR, skylineFPR, axins, 'fpr')
    axins.set_xlim(6, 9.5)
    axins.set_ylim(6e-3, 0.15)
    format_axins(axins, 'linear', 'log')
    patch, pp1, pp2 = plot_inset(ax0, axins, loc1a=2, loc1b=3, loc2a=1, loc2b=4, fc="none", ec="0.5", lw=0.2)

    axins = zoomed_inset_axes(ax0, 2, loc='upper right', borderpad=0)
    # axins = inset_axes(ax0, 0.25, 0.25, loc='lower left', bbox_to_anchor=(0, 0)) # no zoom
    plotFPR(configFingerprintFPR, skylineFPR, axins, 'fpr')
    axins.set_xlim(20, 24)
    axins.set_ylim(5e-7, 2e-5)
    format_axins(axins, 'linear', 'log')
    patch, pp1, pp2 = plot_inset(ax0, axins, loc1a=3, loc1b=2, loc2a=4, loc2b=1, fc="none", ec="0.5")

    ax1 = fig.add_subplot(122)
    handles = plotFPR(configFingerprintK, skylineFPR, ax1, 'k')
    ax1.set_ylabel("Optimal $k$")
    format_axes(ax1)
    ax1.set_xticks([5, 10, 15, 20, 25])
    ax1.set_yticks([5, 10, 15, 20])

    axins = zoomed_inset_axes(ax1, 2, loc='upper left', bbox_to_anchor=(0, 1.05), bbox_transform=ax1.transAxes)
    # axins = inset_axes(ax0, 0.25, 0.25, loc='lower left', bbox_to_anchor=(0, 0)) # no zoom
    plotFPR(configFingerprintK, skylineFPR, axins, 'k')
    axins.set_xlim(6, 9.5)
    axins.set_ylim(3.5, 8.5)
    format_axins(axins, 'linear', 'linear')
    axins.yaxis.set_minor_locator(matplotlib.ticker.FixedLocator([7.5]))
    axins.set_yticks([5])
    for tic in axins.yaxis.get_minor_ticks():
        tic.tick1line.set_visible(False)
    patch, pp1, pp2 = plot_inset(ax1, axins, loc1a=3, loc1b=2, loc2a=4, loc2b=1, fc="none", ec="0.5", lw=0.2)

    axins = zoomed_inset_axes(ax1, 2, loc='lower right', bbox_to_anchor=(1.04, -0.02), bbox_transform=ax1.transAxes)
    # axins = inset_axes(ax0, 0.25, 0.25, loc='lower left', bbox_to_anchor=(0, 0)) # no zoom
    plotFPR(configFingerprintK, skylineFPR, axins, 'k')
    axins.set_xlim(20, 24)
    axins.set_ylim(16.5, 21.5)
    format_axins(axins, 'linear', 'linear')
    axins.yaxis.set_minor_locator(matplotlib.ticker.FixedLocator([17.5]))
    axins.set_yticks([20])
    for tic in axins.yaxis.get_minor_ticks():
        tic.tick1line.set_visible(False)
    patch, pp1, pp2 = plot_inset(ax1, axins, loc1a=2, loc1b=3, loc2a=1, loc2b=4, fc="none", ec="0.5")

    handles[0], handles[1] = handles[1], handles[0]
    allfig = plt.gcf()
    allfig.legend(handles=handles, bbox_to_anchor=(0.5, 1.05), loc='upper center', ncol=4, borderaxespad=0, frameon=False)
    plt.tight_layout()
    savefig("./pdf/background/fingerprintbased_fpr")

if True:
    fig = plt.figure()
    ax0 = fig.add_subplot(121)
    plotFailure(configFingerprintK, skylineFailureK, ax0, True)
    ax0.set_ylabel("Minimal scale factor $s$")
    ax0.set_xlabel("Fingerprint size $k$ [bits]")
    ax0.set_ylim(1.0, 1.62)
    ax0.set_xlim(0, 32.5)
    format_axes(ax0)
    ax0.set_yticks([1, 1.2, 1.4, 1.6])

    ax1 = fig.add_subplot(122)
    handles = plotFailure(configFingerprintK, skylineFailureElements, ax1, False)
    ax1.set_ylim(1.0, 1.62)
    ax1.set_xlabel("Number of keys $n$ (log scale)")
    ax1.set_xlim(1e2, 1.5e7)
    format_axes(ax1, 'log', 'linear')
    ax1.set_xticks([1e2, 1e3, 1e4, 1e5, 1e6, 1e7])
    ax1.set_yticks([1, 1.2, 1.4, 1.6])

    # allfig = plt.gcf()
    # allfig.legend(handles=handles, bbox_to_anchor=(0.5, 1.05), loc='upper center', ncol=4, borderaxespad=0, frameon=False, columnspacing=2, handlelength=1)
    plt.tight_layout()
    savefig("./pdf/background/fingerprintbased_failure")
    plt.close()

skylineAddressing = {}

analyzePerKey(read_benchmark('../benchmark/paper/optimization/addressing/addressing_construct.csv'), skylineAddressing)
analyzePerKey(read_benchmark('../benchmark/paper/optimization/addressing/addressing_count.csv'), skylineAddressing)

for name in {'BloomBlocked', 'Cuckoo', 'Xor', 'Morton'}:
    for fixture in {'Construct', 'Count'}:
        for n_elements in skylineAddressing[f'{name}PowerOfTwo'][fixture]:
            powerOfTwo = skylineAddressing[f'{name}PowerOfTwo'][fixture][n_elements]
            for addr in {'Magic', 'Lemire'}:
                if n_elements in skylineAddressing[f'{name}{addr}'][fixture]:
                    relative = skylineAddressing[f'{name}{addr}'][fixture][n_elements]
                    for attribute in {'t', 'dtlb', 'l1d', 'llc', 'throughput'}:
                        if relative[attribute] == 0 or math.isnan(relative[attribute]):
                            relative[f'{attribute}_speedup'] = -2
                        else:
                            relative[f'{attribute}_speedup'] = powerOfTwo[attribute] / relative[attribute] - 1

config = {
    'MortonLemire': {'label': 'Morton filter', 'color': colors['morton'], 'linestyle': 'solid', 'linewidth': 1},
    'BloomBlockedLemire': {'label': 'Bloom filter', 'color': colors['bloom'], 'linestyle': 'solid', 'linewidth': 1},
    'CuckooLemire': {'label': 'Cuckoo filter', 'color': colors['cuckoo'], 'linestyle': 'solid', 'linewidth': 1},
    'XorLemire': {'label': 'Xor filter', 'color': colors['xor'], 'linestyle': 'solid', 'linewidth': 1},
}

latexify(cm2inch(9), cm2inch(3.5), 2)
fig = plt.figure()
ax = fig.add_subplot(121)
plotFilterSize(config, 'Construct', skylineAddressing, ax, 't_speedup', 1e2, 1.1e5, -0.62)
ax.set_title('\\textbf{Build}', pad=1)
ax.set_ylabel("Speedup [%]")
ax.set_ylim(-0.75, 1.26)
ax.set_xlim(1e2, 1.2e5)
format_axes(ax, 'log', 'linear')
ax.set_xticks([1e2, 1e3, 1e4, 1e5])
ax.set_yticks([-0.5, 0, 0.5, 1])
ax.set_yticklabels(["-50\%", "0\%", "+50%", "~~~~+100\%"], rotation=90, verticalalignment='center')

ax = fig.add_subplot(122)
plotFilterSize(config, 'Count', skylineAddressing, ax, 't_speedup', 1e2, 1.1e5, -0.62)
ax.set_title('\\textbf{Lookup}', pad=1)
ax.set_ylim(-0.75, 1.26)
ax.set_xlim(1e2, 1.2e5)
format_axes(ax, 'log', 'linear')
ax.set_xticks([1e2, 1e3, 1e4, 1e5])
ax.set_yticks([-0.5, 0, 0.5, 1])
ax.set_yticklabels(["-50\%", "0\%", "+50%", "~~~~+100\%"], rotation=90, verticalalignment='center')

handles, labels = ax.get_legend_handles_labels()
handles[0], handles[1], handles[2] = handles[1], handles[2], handles[0]
labels[0], labels[1], labels[2] = labels[1], labels[2], labels[0]
allfig = plt.gcf()
allfig.legend(handles, labels, ncol=4, bbox_to_anchor=(0.5, 1.0), loc='upper center', borderaxespad=0, frameon=False, columnspacing=1)

plt.tight_layout()
savefig("./pdf/optimization/addressing")
plt.close()

skylineHashing = {}

analyzeHashing(read_benchmark('../benchmark/paper/optimization/hashing/hashing_count.csv'), skylineHashing)
analyzeHashing(read_benchmark('../benchmark/paper/optimization/hashing/hashing_construct.csv'), skylineHashing)

config = {
    'Murmur': {'label': 'murmur', 'color1': colors['blue'], 'color2': colors['lightblue'], 'factor': -1},
    'Mul': {'label': 'mul', 'color1': colors['orange'], 'color2': colors['lightorange'], 'factor': 1},
}

latexify(cm2inch(8.5), cm2inch(2), 2)

fig = plt.figure()
ax = fig.add_subplot(111)
ax.axhline(y=0, color='k', linestyle='-', lw=1, zorder=20)
ax.set_ylabel("Speedup [\%]")
handles = plotHashing(config, skylineHashing, ax)
format_axes(ax)
ax.set_yticks([0, 0.25, 0.5, 0.75])
ax.set_ylim(-0, 0.77)
ax.set_xticks([0, 1, 2, 3])
ax.set_xticklabels(["\\textbf{Bloom}", "\\textbf{Cuckoo}", "\\textbf{Morton}", "\\textbf{Xor}"])
ax.tick_params(axis='x', which='major', pad=2.5)
ax.yaxis.set_major_formatter(speedup)
legend = ax.legend((handles['Murmur'], handles['Mul']), ('MurmurMix', 'Mul'), labelspacing=0, ncol=2, bbox_to_anchor=(0.99, 1), borderaxespad=0, framealpha=1, edgecolor='black', fancybox=False)
legend.get_frame().set_linewidth(0.5)
# allfig = plt.gcf()
# allfig.legend((handles['Murmur'], handles['Mul']), ('Murmur', 'Mul'), ncol=2, bbox_to_anchor=(0.5, 1.05), loc='upper center', borderaxespad=0, frameon=False, columnspacing=2)
savefig("./pdf/optimization/hashing")
plt.close()

skylinePartitioning = {}

analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_construct.csv'), skylinePartitioning)
analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_construct_small.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_construct_large.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count_small.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count_large.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count_huge1.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count_huge2.csv'), skylinePartitioning)
# analyzePerKey(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count_huge3.csv'), skylinePartitioning)

config = {
    'BloomBlocked': {'label': '', 'color': colors['bloom'], 'linestyle': 'dashed', 'linewidth': 1},
    'Morton': {'label': '', 'color': colors['morton'], 'linestyle': 'dashed', 'linewidth': 1},
    'Cuckoo': {'label': '', 'color': colors['cuckoo'], 'linestyle': 'dashed', 'linewidth': 1},
    'Xor': {'label': '', 'color': colors['xor'], 'linestyle': 'dashed', 'linewidth': 1},
    'BloomBlockedPartitioned': {'label': 'Bloom filter', 'color': colors['bloom'], 'linestyle': 'solid', 'linewidth': 1},
    'MortonPartitioned': {'label': 'Morton filter', 'color': colors['morton'], 'linestyle': 'solid', 'linewidth': 1},
    'CuckooPartitioned': {'label': 'Cuckoo filter', 'color': colors['cuckoo'], 'linestyle': 'solid', 'linewidth': 1},
    'XorPartitioned': {'label': 'Xor filter', 'color': colors['xor'], 'linestyle': 'solid', 'linewidth': 1},
}

latexify(cm2inch(9), cm2inch(3.1), 2)
fig = plt.figure()
ax = fig.add_subplot(121)
plotFilterSize(config, 'Construct', skylinePartitioning, ax, 't', 1e1, 1.3e6, 223)
ax.set_title('\\textbf{Build}')
ax.set_ylabel("Exection time per key [ns]")
ax.set_ylim(0, 252)
ax.set_xlim(1e1, 1.4e6)
format_axes(ax, 'log', 'linear')
ax.set_xticks([1e1, 1e2, 1e3, 1e4, 1e5, 1e6])
ax.set_yticks([0, 100, 200])
ax.set_yticklabels(["0", "100", "200"], rotation=90, verticalalignment='center')

ax = fig.add_subplot(122)
plotFilterSize(config, 'Count', skylinePartitioning, ax, 't', 1e1, 1.3e6, 78)
ax.set_title('\\textbf{Lookup}')
ax.set_ylim(0, 88)
ax.set_xlim(1e1, 1.4e6)
format_axes(ax, 'log', 'linear')
ax.set_xticks([1e1, 1e2, 1e3, 1e4, 1e5, 1e6])
ax.set_yticks([0, 25, 50, 75])
ax.set_yticklabels(["0", "25", "50", "75"], rotation=90, verticalalignment='center')

handles, labels = ax.get_legend_handles_labels()
handles[1], handles[2] = handles[2], handles[1]
labels[1], labels[2] = labels[2], labels[1]
allfig = plt.gcf()
allfig.legend(handles, labels, ncol=4, bbox_to_anchor=(0.5, 1.02), loc='upper center', borderaxespad=0, frameon=False, columnspacing=1)

plt.tight_layout()
savefig("./pdf/optimization/partitioning_t")

latexify(cm2inch(9), cm2inch(2.6), 2)
fig = plt.figure()
ax = fig.add_subplot(121)
plotFilterSize(config, 'Construct', skylinePartitioning, ax, 'dtlb', 1e1, 1.3e6, 7.1, True)
ax.set_ylabel("dTLB-misses per key")
ax.yaxis.set_label_coords(-0.1, 0.35)
ax.set_ylim(-0.25, 7.75)
ax.set_xlim(1e1, 1.4e6)
format_axes(ax, 'log', 'linear')
ax.set_xticks([1e1, 1e2, 1e3, 1e4, 1e5, 1e6])
ax.set_yticks([0, 2, 4, 6])

ax = fig.add_subplot(122)
plotFilterSize(config, 'Count', skylinePartitioning, ax, 'llc', 1e1, 1.3e6, 2.99)
ax.set_ylabel("LLC-misses per key")
ax.yaxis.set_label_coords(-0.1, 0.35)
ax.set_ylim(0, 3.25)
ax.set_xlim(1e1, 1.4e6)
format_axes(ax, 'log', 'linear')
ax.set_xticks([1e1, 1e2, 1e3, 1e4, 1e5, 1e6])
ax.set_yticks([0, 1, 2, 3])

plt.tight_layout()
savefig("./pdf/optimization/partitioning_misses")

skylinePartition = {}

analyzeCorridor(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count.csv'), skylinePartition)
analyzeCorridor(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_construct.csv'), skylinePartition)

skylineCorridor = {}

for name in {'BloomBlocked', 'Cuckoo', 'Xor'}:
    skylineCorridor[name] = {'Count': {}, 'Construct': {}}

    for fixture in ['Count', 'Construct']:
        partitionName = f'{name}Partitioned'
        for n_elements in skylinePartition[partitionName][fixture]:
            ts = {}  # all thoughputs
            minT = float('inf')
            minTId = 0
            if 0.0 in skylinePartition[name][fixture][n_elements]:
                skylinePartition[partitionName][fixture][n_elements][0.0] = skylinePartition[name][fixture][n_elements][0.0]
            for partitionNum in skylinePartition[partitionName][fixture][n_elements]:
                partitioned = skylinePartition[partitionName][fixture][n_elements][partitionNum]
                ts[partitionNum] = partitioned['t']
                if partitioned['t'] < minT:
                    minTId = partitionNum
                    minT = partitioned['t']
            skylineCorridor[name][fixture][n_elements] = {}
            skylineCorridor[name][fixture][n_elements]['size'] = skylinePartition[partitionName][fixture][n_elements][minTId]['size']
            skylineCorridor[name][fixture][n_elements]['min'] = minTId
            skylineCorridor[name][fixture][n_elements]['corridor'] = []
            skylineCorridor[name][fixture][n_elements]['corridor5'] = []

            for partitionNum in ts:
                t = ts[partitionNum]
                if t < (minT / 0.9):
                    skylineCorridor[name][fixture][n_elements]['corridor'].append(partitionNum)

                if t < (minT / 0.95):
                    skylineCorridor[name][fixture][n_elements]['corridor5'].append(partitionNum)

configConstruct = {
    'BloomBlocked': {'label': 'bloom', 'color': colors['bloom'], 'linestyle': 'solid', 'linewidth': 1},
    'Cuckoo': {'label': 'cuckoo', 'color': colors['cuckoo'], 'linestyle': 'solid', 'linewidth': 1},
    'Xor': {'label': 'xor', 'color': colors['xor'], 'linestyle': 'solid', 'linewidth': 1},
}

configCount = {
    'BloomBlocked': {'label': 'bloom', 'color': colors['bloom'], 'linestyle': 'solid', 'linewidth': 1},
    'Cuckoo': {'label': 'cuckoo', 'color': colors['cuckoo'], 'linestyle': 'solid', 'linewidth': 1},
    #    'Xor': {'label': 'xor', 'color': colors['xor'], 'linestyle': 'solid', 'linewidth': 1},
}

latexify(cm2inch(8.5), cm2inch(3.5), 2)

fig = plt.figure()
ax = fig.add_subplot(121)
plotCorridor(configConstruct, skylineCorridor, ax, 'Construct', 1e1, 1.3e6, 12.92)
format_axes(ax, 'log', 'linear')
ax.set_xlim(1e1, 1.4e6)
ax.set_ylim(-0.5, 14.5)
ax.set_ylabel("Number of Partitions", labelpad=4)
format_axes(ax, 'log', 'linear')
ax.set_title('\\textbf{Build}')
ax.set_yticks([0, 4, 8, 12])
ax.set_xticks([1e1, 1e2, 1e3, 1e4, 1e5, 1e6])
ax.set_yticklabels(["$0$", "$2^{4}$", "$2^{8}$", "$2^{12}$"], horizontalalignment="left", x=-0.075)

ax = fig.add_subplot(122)
plotCorridor(configCount, skylineCorridor, ax, 'Count', 1e1, 1.3e6, 12.92)
format_axes(ax, 'log', 'linear')
ax.set_xlim(1e1, 1.4e6)
ax.set_ylim(-0.5, 14.5)
format_axes(ax, 'log', 'linear')
ax.set_yticks([0, 4, 8, 12])
ax.set_yticklabels([])
ax.set_xticks([1e1, 1e2, 1e3, 1e4, 1e5, 1e6])
ax.set_title('\\textbf{Lookup}')

plt.tight_layout()
savefig("./pdf/optimization/partitioning_corridor")

skylineSIMD = {}

analyzeSIMD(read_benchmark('../benchmark/paper/optimization/partitioning/partitioning_count.csv'), skylineSIMD)
analyzeSIMD(read_benchmark('../benchmark/paper/optimization/simd/simd_count.csv'), skylineSIMD)

for name in {'BloomBlocked', 'Cuckoo', 'Morton', 'Xor'}:
    for n_elements in skylineSIMD[f'{name}SIMD']['Count']:
        simd = skylineSIMD[f'{name}SIMD']['Count'][n_elements]
        base_name = name.replace('Horizontal', '').replace('Vertical', '')
        if n_elements in skylineSIMD[base_name]['Count']:
            scalar = skylineSIMD[base_name]['Count'][n_elements]
            relative = {'size': simd['size']}
            simd['speedup'] = -2 if simd['t'] == 0 or math.isnan(simd['t']) else scalar['t'] / simd['t'] - 1

config = {
    'MortonSIMD': {'label': 'Morton filter', 'color': colors['morton'], 'linestyle': 'solid', 'linewidth': 1},
    'CuckooSIMD': {'label': 'Cuckoo filter', 'color': colors['cuckoo'], 'linestyle': 'solid', 'linewidth': 1},
    'BloomBlockedSIMD': {'label': 'Bloom filter', 'color': colors['bloom'], 'linestyle': 'solid', 'linewidth': 1},
    'XorSIMD': {'label': 'Xor filter', 'color': colors['xor'], 'linestyle': 'solid', 'linewidth': 1},
}

latexify(cm2inch(8.5), cm2inch(2.5), 2)

fig = plt.figure()
ax = fig.add_subplot(111)
handles = plotFilterSize(config, 'Count', skylineSIMD, ax, 'speedup', 1e0, 1.3e6, 0.95)
format_axes(ax, 'log', 'linear')
ax.set_ylabel("Speedup [%]")
ax.set_xlim(1e0, 1.4e6)
ax.set_ylim(-0.15, 1.05)
ax.yaxis.set_major_formatter(speedup)
handles, labels = ax.get_legend_handles_labels()
handles[0], handles[2] = handles[2], handles[0]
labels[0], labels[2] = labels[2], labels[0]
allfig = plt.gcf()
allfig.legend(handles, labels, ncol=4, bbox_to_anchor=(0.45, 1.04), loc='upper center', borderaxespad=0, frameon=False, columnspacing=1)
savefig("./pdf/optimization/simd")

skylineMultiThreading = {}

analyzeMultiThreading('Skylake_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_count_9900X.csv'), skylineMultiThreading)
analyzeMultiThreading('Skylake_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_mtcount_9900X.csv'), skylineMultiThreading)
analyzeMultiThreading('Skylake_Construct', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_construct_9900X.csv'), skylineMultiThreading)

analyzeMultiThreading('Ryzen_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_count_ryzen3000.csv'), skylineMultiThreading)
analyzeMultiThreading('Ryzen_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_mtcount_ryzen3000.csv'), skylineMultiThreading)
analyzeMultiThreading('Ryzen_Construct', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_construct_ryzen3000.csv'), skylineMultiThreading)

analyzeMultiThreading('Ryzen_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_count_ryzen3000_numa.csv'), skylineMultiThreading, True)
analyzeMultiThreading('Ryzen_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_mtcount_ryzen3000_numa.csv'), skylineMultiThreading, True)
analyzeMultiThreading('Ryzen_Construct', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_construct_ryzen3000_numa.csv'), skylineMultiThreading, True)

analyzeMultiThreading('Xeon_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_count_xeon.csv'), skylineMultiThreading)
analyzeMultiThreading('Xeon_Count', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_mtcount_xeon.csv'), skylineMultiThreading)
analyzeMultiThreading('Xeon_Construct', read_benchmark('../benchmark/paper/optimization/multi-threading/multi_threading_construct_xeon.csv'), skylineMultiThreading)

for machine in {'Skylake', 'Xeon', 'Ryzen'}:
    for filtername in {'BloomBlocked', 'Cuckoo', 'Morton', 'Xor'}:
        for name in {f'{machine}_Count_{filtername}MT', f'{machine}_Count_{filtername}PartitionedMT', f'{machine}_Count_{filtername}MTPartitioned', f'{machine}_Construct_{filtername}PartitionedMT'}:
            if name in skylineMultiThreading:
                singlethreaded = skylineMultiThreading[name.replace('MT', '')][1]
                for n_threads in skylineMultiThreading[name]:
                    multithreaded = skylineMultiThreading[name][n_threads]
                    multithreaded['scaleup'] = 0 if multithreaded['t'] == 0 or math.isnan(multithreaded['t']) else singlethreaded['t'] / multithreaded['t']

config = {
    'Construct_BloomBlockedPartitionedMT': {'label': 'Build\\textsuperscript{Part+MT}', 'color': colors['bloom'], 'linestyle': 'solid', 'linewidth': 1},
    'Count_BloomBlockedMT': {'label': 'Lookup\\textsuperscript{MT}', 'color': colors['cuckoo'], 'linestyle': 'solid', 'linewidth': 1},
    'Count_BloomBlockedPartitionedMT': {'label': 'Lookup\\textsuperscript{Part+MT}', 'color': colors['morton'], 'linestyle': 'solid', 'linewidth': 1},
    'Count_BloomBlockedMTPartitioned': {'label': 'Lookup\\textsuperscript{MT+Part}', 'color': colors['xor'], 'linestyle': 'solid', 'linewidth': 1},
}


def format(ax, title):
    ax.set_xlabel("Threads")
    format_axes(ax, 'linear', 'linear')
    ax.set_ylabel("Scale-Up")
    ax.set_title(f'\\textbf{"{" + title + "}"}', pad=1)
    ax.legend().set_visible(False)
    ax.set_ylim(1)

    if title == 'Skylake-X':
        ax.set_xlim(0, 14.25)
        ax.xaxis.set_major_locator(matplotlib.ticker.FixedLocator([0, 1, 3, 5, 7, 9, 11.5, 14]))
        ax.set_xticklabels([1, 2, 4, 6, 8, 10, 15, 20])
        ax.yaxis.set_major_locator(matplotlib.ticker.FixedLocator([1, 5, 10]))
        ax.yaxis.set_minor_locator(matplotlib.ticker.FixedLocator([3, 7.5]))
        ind = np.arange(15)
        ylim = ax.get_ylim()
        ax.add_patch(Rectangle((ind[-6] + .01, ylim[0]), 8, ylim[1] - ylim[0], color='#000000', alpha=0.15, linewidth=0))
        ax.text(9 + 5.25 * 0.5, ylim[0] + (ylim[1] - ylim[0]) * .04, '\\textsf{\\textbf{SMT}}', ha='center', va='bottom', color='w')
    if title == 'Ryzen':
        ax.set_xlim(0, 19.30)
        ax.set_ylim(1, 16.1)
        ax.xaxis.set_major_locator(matplotlib.ticker.FixedLocator([0, 3, 7, 11, 15, 17, 19]))
        ax.set_xticklabels([1, 4, 8, 12, 16, 24, 32])
        ax.yaxis.set_major_locator(matplotlib.ticker.FixedLocator([1, 8, 16]))
        ax.yaxis.set_minor_locator(matplotlib.ticker.FixedLocator([4.5, 12]))
        ind = np.arange(20)
        ylim = ax.get_ylim()
        ax.add_patch(Rectangle((ind[-5] + .01, ylim[0]), 8, ylim[1] - ylim[0], color='#000000', alpha=0.15, linewidth=0))
        ax.text(15 + 4.3 * 0.5, ylim[0] + (ylim[1] - ylim[0]) * .05, '\\textsf{\\textbf{SMT}}', ha='center', va='bottom', color='w')
    if title == 'Xeon Gold':
        ax.set_xlim(0, 27.3)
        ax.xaxis.set_major_locator(matplotlib.ticker.FixedLocator([0, 3, 7, 11, 15, 19, 23, 25, 27]))
        ax.set_xticklabels([1, 4, 8, 12, 16, 20, 24, 36, 48])
        ax.yaxis.set_major_locator(matplotlib.ticker.FixedLocator([1, 12, 24]))
        ax.yaxis.set_minor_locator(matplotlib.ticker.FixedLocator([6.5, 18]))
        ind = np.arange(28)
        ylim = ax.get_ylim()
        ax.add_patch(Rectangle((ind[-5] + .01, ylim[0]), 8, ylim[1] - ylim[0], color='#000000', alpha=0.15, linewidth=0))
        ax.text(23 + 4.3 * 0.5, ylim[0] + (ylim[1] - ylim[0]) * .05, '\\textsf{\\textbf{SMT}}', ha='center', va='bottom', color='w')


latexify(cm2inch(8.5), cm2inch(4.5), 2)

fig = plt.figure()
ax0 = fig.add_subplot(221)
ax1 = fig.add_subplot(222)
ax2 = fig.add_subplot(212)

datapoints = list(range(1, 11)) + [12, 14, 16, 18, 20]
plotScaleup(config, skylineMultiThreading, ax0, 'Skylake_', datapoints)
format(ax0, 'Skylake-X')

datapoints = list(range(1, 17)) + [20, 24, 28, 32]
plotScaleup(config, skylineMultiThreading, ax1, 'Ryzen_', datapoints)
format(ax1, 'Ryzen')

datapoints = list(range(1, 25)) + [30, 36, 42, 48]
plotScaleup(config, skylineMultiThreading, ax2, 'Xeon_', datapoints)
format(ax2, 'Xeon Gold')

handles, labels = ax0.get_legend_handles_labels()
allfig = plt.gcf()
allfig.legend(handles, labels, ncol=4, bbox_to_anchor=(0.5, 1.05), loc='upper center', borderaxespad=0, frameon=False)

plt.tight_layout(h_pad=-0.5)
fig.subplots_adjust(bottom=0.15)
savefig("./pdf/optimization/multithreading")

skylineCompetitors = {}

analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_bsd_construct.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_bsd_count.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_cuckoo_construct.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_cuckoo_count.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_impala_construct.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_impala_count.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_xor_construct.csv'), skylineCompetitors)
analyzeCompetitors(read_benchmark('../benchmark/paper/experiments/competitors/competitors_xor_count.csv'), skylineCompetitors)

config = {
    'BSD_Bloom_Blocked512': {'label': 'cache-blocked\nBloom filter~\cite{Lang19}', 'competitor': 'Bloom_Blocked512'},
    'BSD_Bloom_Blocked32': {'label': 'register-blocked\nBloom filter~\cite{Lang19}', 'competitor': 'Bloom_Blocked32'},
    'BSD_Bloom_Grouped2': {'label': 'cache-sectorized\nBloom filter~\cite{Lang19}', 'competitor': 'Bloom_Grouped2'},
    'Impala_Bloom_Sectorized256_AVX2': {'label': 'sectorized\nBloom filter~\cite{Kornacker15}', 'competitor': 'Bloom_Sectorized256_AVX2'},
    'Efficient_Cuckoo_Standard4_Scalar': {'label': 'Cuckoo filter~\cite{Fan14}', 'competitor': 'Cuckoo_Standard4_Scalar'},
    'AMD_Morton_Standard3_Scalar': {'label': 'Morton filter~\cite{Breslow18}', 'competitor': 'Morton_Standard3_Scalar'},
    'Fastfilter_Xor_Standard_Scalar': {'label': 'Xor filter~\cite{Graf20}', 'competitor': 'Xor_Standard_Scalar'},
}


def plotCompetitors(config, fixture, skyline, ax, legend=True):
    width = 0.4
    ind = []
    label = []
    p = {}
    for fixture in ['Construct', 'Count']:
        for i, name in enumerate(config.keys()):
            ind.append(i)
            label.append(config[name]['label'])
            t_competitor = skyline[name][fixture]['t']
            t_our = skyline[config[name]['competitor']][fixture]['t']
            p['Competitor'] = ax.bar([(-0.01 if fixture == 'Construct' else 0.01) + i + width / 4 * (-3 if fixture == 'Construct' else 1)], [t_competitor], width / 2, color=colors['blue'], zorder=10)
            p['Ours'] = ax.bar([(-0.01 if fixture == 'Construct' else 0.01) + i + width / 4 * (-1 if fixture == 'Construct' else 3)], [t_our], width / 2, color=colors['orange'], zorder=10)

            bar0 = p['Competitor'][0]
            bar1 = p['Ours'][0]
            posText = (bar0.get_height() + bar1.get_height()) / 2
            if t_competitor <= t_our:
                middle = bar0.get_x() + bar0.get_width() / 2
            else:
                middle = bar1.get_x() + bar1.get_width() / 2
            height = max(bar0.get_height(), bar1.get_height())
            ax.plot([bar0.get_x(), bar0.get_x() + bar0.get_width() * 2], [height, height], 'k-', lw=0.5, zorder=20)
            ax.plot([middle, middle], [bar0.get_height(), bar1.get_height()], 'k-', lw=0.5, zorder=20)
            ax.text(bar1.get_x() + (-0.05 if fixture == 'Construct' and t_our / t_competitor - 1 > 0.99 else 0), height + 0.005e9, '\\textbf{' + speedup(t_our / t_competitor - 1) + '}', ha='center',
                    va='bottom', fontsize=6, color='k')
            ax.text(bar0.get_x() + width / 2 + (0.05 if fixture == 'Count' else 0), -4e6, 'Lookup' if fixture == 'Count' else 'Build', ha='center', va='top', color='k', fontsize=4, zorder=20)
            # ax.text(bar1.get_x() + width / 2 + 0.05, -1e6, 'Lookup' if fixture == 'Count' else 'Build', ha='center', va='top', color='k', fontsize=4, zorder=20)

    l = ax.legend((p['Competitor'], p['Ours']), ('Competitor', 'Ours'), labelspacing=0, ncol=2, bbox_to_anchor=(0.99, 1), borderaxespad=0, framealpha=1, edgecolor='black', fancybox=False)
    l.set_visible(legend)
    l.get_frame().set_linewidth(0.5)
    ax.set_xticks(ind)
    ax.set_xticklabels(label, rotation=30, ha='right', rotation_mode="anchor", fontsize=6)
    ax.yaxis.set_major_formatter(gigs)
    ax.set_ylim(0, 0.41e9)
    ax.set_xlim(-0.55, 6.45)
    ax.set_yticks([0, 0.2e9, 0.4e9])
    ax.axhline(y=0, color='k', linestyle='-', lw=1, zorder=20)


latexify(cm2inch(8.5), cm2inch(4), 2)

# 6
latexify(cm2inch(9), cm2inch(6.5), 1)
fig = plt.figure()
ax = fig.add_subplot(211)
format_axes(ax)
plotCompetitors(config, 'Count', skylineCompetitors, ax)
# ax.set_xticklabels([])
# ax.set_title('\\textbf{Lookup}')
ax.set_ylabel("Throughput [Keys/s]")

# ax = fig.add_subplot(212)
# format_axes(ax)
# plotCompetitors(config, 'Construct', skylineCompetitors, ax, False)
# ax.set_title('\\textbf{Build}')
# ax.set_ylabel("Throughput [Keys/s]")

plt.tight_layout()
fig.subplots_adjust(bottom=0.22)  # or whatever
savefig("./pdf/experiments/competitors")
