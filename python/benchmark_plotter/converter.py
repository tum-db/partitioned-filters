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
