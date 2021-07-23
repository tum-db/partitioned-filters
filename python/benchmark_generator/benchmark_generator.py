import argparse
import copy
import json
import math
import os

from jsonschema import validate
from typing import Any
from jinja2 import Template

SCHEMA_PATH = 'benchmark/benchmark.schema.json'
TEMPLATE_PATH = 'benchmark/benchmark.cpp.template'


def preprocess_s(benchmark: Any) -> [Any]:
    min_bits = benchmark['parameter']['s'][0]
    max_bits = benchmark['parameter']['s'][1]
    step_bits = benchmark['parameter']['s'][2]
    min_s = benchmark['parameter']['s'][3]
    max_s = benchmark['parameter']['s'][4]

    benchmarks = []
    for k in benchmark['parameter']['k']:
        new_benchmark = copy.deepcopy(benchmark)
        new_benchmark['parameter']['k'] = [k]
        new_benchmark['parameter']['s'] = []

        bits = min_bits
        while bits <= max_bits:
            s = int(bits / k)
            if min_s <= s <= max_s and s not in new_benchmark['parameter']['s']:
                new_benchmark['parameter']['s'].append(s)
            bits += step_bits

        if len(new_benchmark['parameter']['s']) > 0:
            benchmarks.append(new_benchmark)

    return benchmarks


def preprocess_s_morton(benchmark: Any) -> [Any]:
    type = benchmark['filter']['type']
    variant = benchmark['filter']['variant']

    assert (type == 'Cuckoo')
    assert (variant.startswith('Morton'))
    variant = variant.replace('Morton', '')
    fingerprints_per_block = int(variant.split('_')[0])
    compression_rate = int(variant.split('_')[1])
    ota_rate = int(variant.split('_')[2])

    ks = benchmark['parameter']['k']
    max_s = benchmark['parameter']['s'][3]
    benchmarks = []

    for k in ks:
        min_s = 1 + (math.ceil(math.log(fingerprints_per_block + 1, 2)) * 100 + ota_rate) / (
                fingerprints_per_block * compression_rate * k)
        new_benchmark = copy.deepcopy(benchmark)
        new_benchmark['parameter']['k'] = [k]
        new_benchmark['parameter']['s'][3] = int(min_s * 100)
        new_benchmark['parameter']['s'].append(max_s)
        benchmarks += preprocess_s(new_benchmark)

    return benchmarks


def preprocess_n_elements_log(benchmark: Any) -> [Any]:
    base_build = benchmark['parameter']['n_elements'][0]['build']
    base_lookup = benchmark['parameter']['n_elements'][0]['lookup']
    min_build = benchmark['parameter']['n_elements'][1]['build']
    min_lookup = benchmark['parameter']['n_elements'][1]['lookup']
    max_build = benchmark['parameter']['n_elements'][2]['build']
    max_lookup = benchmark['parameter']['n_elements'][2]['lookup']
    step_build = benchmark['parameter']['n_elements'][3]['build'] / 100
    step_lookup = benchmark['parameter']['n_elements'][3]['lookup'] / 100
    min_elements_build = benchmark['parameter']['n_elements'][4]['build']
    min_elements_lookup = benchmark['parameter']['n_elements'][4]['lookup']
    shared = benchmark['parameter']['n_elements'][0]['shared']

    benchmark['parameter']['n_elements'] = []
    exp_build = min_build
    exp_lookup = min_lookup
    while True:
        build = max(pow(base_build, exp_build), min_elements_build)
        lookup = max(pow(base_lookup, exp_lookup), min_elements_lookup)
        benchmark['parameter']['n_elements'].append({'build': round(build), 'lookup': round(lookup), 'shared': shared})
        exp_build += step_build
        exp_lookup += step_lookup
        if (step_build == 0 or exp_build > max_build) and (step_lookup == 0 or exp_lookup > max_lookup):
            break
        else:
            exp_build = min(max_build, exp_build)
            exp_lookup = min(max_lookup, exp_lookup)

    return [benchmark]


def preprocess_s_and_n_elements_log(benchmark: Any) -> [Any]:
    benchmarks = []
    for b in preprocess_s(benchmark):
        benchmarks.extend(preprocess_n_elements_log(b))

    return benchmarks


def parse_args() -> Any:
    parser = argparse.ArgumentParser(description='Generator Benchmark')
    parser.add_argument('json', type=str, help='json file')
    parser.add_argument('cpp', type=str, help='cpp file')
    return parser.parse_args()


def read_benchmark(path: str) -> Any:
    with open(SCHEMA_PATH) as schema_file:
        with open(path) as json_file:
            schema = json.load(schema_file)
            definition = json.load(json_file)
            validate(instance=definition, schema=schema)
            return definition


def merge(object1: Any, object2: Any, key: str):
    if key not in object1:
        object1[key] = copy.deepcopy(object2[key])
    else:
        if isinstance(object2[key], dict):
            for subkey in object2[key].keys():
                object1[key] = merge(object1[key], object2[key], subkey)
    return object1


def normalize_benchmark(definition: Any) -> Any:
    benchmarks = []
    for benchmark in definition['benchmarks']:
        benchmark = merge(benchmark, definition, 'optimization')
        benchmark = merge(benchmark, definition, 'parameter')
        if isinstance(benchmark['parameter']['n_elements'][0], int):
            n_elements = []
            for n_elements_build in benchmark['parameter']['n_elements']:
                n_elements.append({'build': n_elements_build, 'lookup': 0, 'shared': 0})
            benchmark['parameter']['n_elements'] = n_elements
        benchmarks.append(benchmark)
    definition['benchmarks'] = benchmarks
    return definition


def preprocess_benchmark(definition: Any) -> Any:
    if 'preprocess' not in definition:
        return definition

    benchmarks = []
    for benchmark in definition['benchmarks']:
        if definition['preprocess'] == 'preprocess_s':
            benchmarks += preprocess_s(benchmark)
        elif definition['preprocess'] == 'preprocess_n_elements_log':
            benchmarks += preprocess_n_elements_log(benchmark)
        elif definition['preprocess'] == 'preprocess_s_morton':
            benchmarks += preprocess_s_morton(benchmark)
        elif definition['preprocess'] == 'preprocess_s_and_n_elements_log':
            benchmarks += preprocess_s_and_n_elements_log(benchmark)

    definition['benchmarks'] = benchmarks
    return definition


def write_benchmark(path: str, definition: Any):
    if not os.path.exists(os.path.dirname(path)):
        os.makedirs(os.path.dirname(path))

    with open(TEMPLATE_PATH) as template_file:
        with open(path, 'w') as cpp_file:
            template = Template(template_file.read(), trim_blocks=True, lstrip_blocks=True)
            cpp_file.write(template.render(definition))


args = parse_args()
definition = read_benchmark(args.json)
definition = normalize_benchmark(definition)
definition = preprocess_benchmark(definition)
write_benchmark(args.cpp, definition)
