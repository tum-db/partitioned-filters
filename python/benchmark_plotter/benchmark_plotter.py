import json
import os
import numpy as np

import matplotlib
import matplotlib.pyplot as plt

from colors import colors
from converter import yconverter, xscale
from latexify import cm2inch, latexify, format_axes
from markers import markers
from read_benchmark import read_benchmark

BENCHMARK_DEFINITION_PATH = 'benchmark/paper'


def find_all_benchmarks() -> [dict]:
    benchmarks = []
    for root, dirs, files in os.walk(BENCHMARK_DEFINITION_PATH):
        for file in files:
            if file.endswith(".json"):
                path = os.path.join(root, file)
                with open(path) as json_file:
                    data = json.load(json_file)
                    name = data['name']
                    directory = os.path.dirname(path)
                    filename = os.path.splitext(os.path.basename(path))[0]
                    csv = f'{directory}/{filename}.csv'

                    if os.path.isfile(csv) and data['visualization']['enable']:
                        os.makedirs(f'{directory}/{filename}_plot/', exist_ok=True)
                        benchmarks.append({'name': name, 'path': path, 'data': data, 'csv': csv,
                                           'image_prefix': f'{directory}/{filename}_plot/'})

    return benchmarks


def print_benchmark_menu(benchmarks: [dict]):
    print('(0) All')
    for i in range(len(benchmarks)):
        print(f'({i + 1}) {benchmarks[i]["name"]} - {benchmarks[i]["path"]}')


def plot_line(data: dict, meta: dict, title: str, impage_prefix: str, xaxis: str, yaxis: str):
    latexify(cm2inch(8.5), cm2inch(7), 1)
    fig = plt.figure()
    ax0 = fig.add_subplot(111)

    for name in data.keys():
        x = []
        y = []

        for index in range(len(data[name])):
            x.append(data[name][index][xaxis])
            y.append(yconverter[yaxis](data[name], index, xaxis, yaxis))

        ax0.plot(x, y, label=name, color=meta[name]['color'], marker=meta[name]['marker'], linewidth=1, markersize=2)

    ax0.yaxis.set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax0.grid(b=True, which='minor', linestyle=':')
    ax0.set_title(title)
    ax0.set_ylabel(yaxis.replace('_', '\_'))
    ax0.set_xlabel(xaxis.replace('_', '\_'))
    ax0.set_xscale(xscale[xaxis])
    ax0.set_ylim(0)
    ax0.legend().set_visible(False)
    format_axes(ax0)

    handles, labels = ax0.get_legend_handles_labels()
    allfig = plt.gcf()
    allfig.legend(handles, labels, bbox_to_anchor=(0.5, 0.4), ncol=2, loc='upper center', borderaxespad=0)
    plt.tight_layout(h_pad=0.5, w_pad=0)
    fig.subplots_adjust(bottom=0.5)
    fig.savefig(os.path.join(impage_prefix, f'{yaxis}.pdf'), bbox_inches='tight', pad_inches=0)
    plt.close()


def plot_bar(data: dict, meta: dict, title: str, impage_prefix: str, yaxis: str):
    latexify(cm2inch(8.5), cm2inch(7), 1)
    fig = plt.figure()
    ax0 = fig.add_subplot(111)

    for i, name in enumerate(data.keys()):
        assert (len(data[name]) == 1)
        ax0.bar(i, yconverter[yaxis](data[name], 0, name, yaxis), color=meta[name]['color'], label=name)

    ax0.yaxis.set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax0.grid(b=True, which='minor', linestyle=':')
    ax0.set_title(title)
    ax0.set_ylabel(yaxis.replace('_', '\_'))
    ax0.set_ylim(0)
    ax0.legend().set_visible(False)
    format_axes(ax0)

    handles, labels = ax0.get_legend_handles_labels()
    allfig = plt.gcf()
    allfig.legend(handles, labels, bbox_to_anchor=(0.5, 0.4), ncol=2, loc='upper center', borderaxespad=0)
    plt.tight_layout(h_pad=0.5, w_pad=0)
    fig.subplots_adjust(bottom=0.5)
    fig.savefig(os.path.join(impage_prefix, f'{yaxis}.pdf'), bbox_inches='tight', pad_inches=0)
    plt.close()


def plot_benchmark(benchmark: dict):
    print(f'Plotting benchmark {benchmark["name"]}')

    chart = benchmark['data']['visualization']['chart']
    if chart == 'line':
        x = benchmark['data']['visualization']['x-axis']
    yaxis = benchmark['data']['visualization']['y-axis']

    meta = {}
    for i, b in enumerate(benchmark['data']['benchmarks']):
        color = colors[list(colors.keys())[i % len(colors)] if 'color' not in b else b['color']]
        marker = markers[list(markers.keys())[int(i / len(colors))] if 'marker' not in b else b['marker']]
        meta[b['name'].replace('_', '\_')] = {'color': color, 'marker': marker}

    data = read_benchmark(benchmark['csv'])

    for y in yaxis:
        if chart == 'line':
            plot_line(data, meta, benchmark['data']['name'], benchmark['image_prefix'], x, y)
        elif chart == 'bar':
            plot_bar(data, meta, benchmark['data']['name'], benchmark['image_prefix'], y)


benchmarks = find_all_benchmarks()
print_benchmark_menu(benchmarks)
index = int(input("Choose a benchmark: "))

assert 0 <= index <= len(benchmarks)
if index == 0:
    for benchmark in benchmarks:
        plot_benchmark(benchmark)
else:
    plot_benchmark(benchmarks[index - 1])
