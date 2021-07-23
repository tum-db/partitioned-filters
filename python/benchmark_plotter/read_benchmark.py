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
