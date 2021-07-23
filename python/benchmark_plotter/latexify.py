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
              'axes.labelsize': 8,  # fontsize for x and y labels (was 10)
              'axes.titlesize': 8,
              'font.size': 8,  # was 10
              'legend.fontsize': 6,  # was 10
              'legend.handlelength': 1.5,
              'legend.handletextpad': 0.3,
              'legend.labelspacing': 0.3, # was 0.1
              'legend.columnspacing': 0.3,
              'legend.borderpad': 0.3,
              'xtick.labelsize': 7,
              'ytick.labelsize': 7,
              'axes.labelpad': 1,
              'axes.titlepad': 2,
              'text.usetex': True,
              'figure.figsize': [fig_width, fig_height],
              'font.family': 'serif',
              'text.latex.preamble': r'\usepackage{amssymb} \usepackage{ifsym}'
              }

    matplotlib.rcParams.update(params)


def format_axes(ax):
    spine_color = 'black'
    for spine in ['top', 'right']:
        ax.spines[spine].set_visible(False)

    for spine in ['left', 'bottom']:
        ax.spines[spine].set_color(spine_color)
        ax.spines[spine].set_linewidth(0.5)

    ax.xaxis.set_ticks_position('bottom')
    ax.yaxis.set_ticks_position('left')

    for axis in [ax.xaxis, ax.yaxis]:
        axis.set_tick_params(direction='out', color=spine_color)

    ax.yaxis.set_minor_locator(AutoMinorLocator(n=2))
    ax.yaxis.grid(True)
    ax.yaxis.grid(b=True, which='minor', linestyle=':')
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
