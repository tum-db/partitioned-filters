import json
import os
import sys

BENCHMARK_DEFINITION_PATH = 'benchmark/paper'


def find_all_benchmarks() -> [dict]:
    benchmarks = []
    for root, dirs, files in os.walk(BENCHMARK_DEFINITION_PATH):
        for file in files:
            if file.endswith(".json"):
                path = os.path.join(root, file)
                with open(path) as json_file:
                    name = json.load(json_file)['name']
                    directory = os.path.dirname(path)
                    filename = os.path.splitext(os.path.basename(path))[0]
                    benchmarks.append({'name': name, 'path': path, 'target': f'benchmark_{filename}',
                                       'csv': f'{directory}/{filename}.csv'})

    return benchmarks


def print_benchmark_menu(benchmarks: [dict]):
    print('(0) All')
    for i in range(len(benchmarks)):
        print(f'({i + 1}) {benchmarks[i]["name"]} - {benchmarks[i]["path"]}')


def build_benchmark(benchmark: dict):
    print(f'Building benchmark {benchmark["name"]}')
    exitCode = os.system(f'cd build; make {benchmark["target"]} -j 10')
    if (exitCode != 0):
        sys.exit(f'Could not build benchmark {benchmark["name"]}!')


def execute_benchmark(benchmark: dict):
    print(f'Executing benchmark {benchmark["name"]}')
    exitCode = os.system(f'./build/benchmark/{benchmark["target"]} --benchmark_format=csv | tee {benchmark["csv"]}')
    if (exitCode != 0):
        sys.exit(f'Could not execute benchmark {benchmark["name"]}!')


benchmarks = sorted(find_all_benchmarks(), key=lambda k: k['name'])
print_benchmark_menu(benchmarks)
print("Use space to separate benchmarks")
rawInput = input("Choose a benchmark: ")

if " " in rawInput:
    inputs = [int(x) for x in rawInput.split(" ")]

    for i in inputs:
        assert 0 < i <= len(benchmarks)
        build_benchmark(benchmarks[i - 1])

    input("Press ENTER to start benchmarks")
    for i in inputs:
        assert 0 < i <= len(benchmarks)
        execute_benchmark(benchmarks[i - 1])

else:
    index = int(rawInput)
    assert 0 <= index <= len(benchmarks)
    if index == 0:
        for benchmark in benchmarks:
            build_benchmark(benchmark)
        input("Press ENTER to start benchmarks")
        for benchmark in benchmarks:
            execute_benchmark(benchmark)
    else:
        build_benchmark(benchmarks[index - 1])
        #input("Press ENTER to start benchmark")
        execute_benchmark(benchmarks[index - 1])
