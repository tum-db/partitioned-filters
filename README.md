# Partitioned Filters

The repository contains our implementations of four approximate filters: the Bloom filter [1], the Cuckoo filter [2], the Morton filter [3], and the Xor filter [4]. We used the code in our paper [A four-dimensional Analysis of Partitioned Filters](https://db.in.tum.de/~bandle/papers/vldb21-partitionedFilters.pdf).

In addition to our optimized filter implementations, the repository also contains the code of state-of-the-art competitors we compare to and extensive test cases. We generate the benchmarks using python scripts and included our results on an Intel i9-9900x (Skylake-X) with 64 GiB memory.

## Using the Code

### Prerequisites

- C++20 (we used GCC 10.2)
- CMake version 3.10 & make
- Linux (we used Ubuntu 20.10 with kernel version 5.8)
- Python3 + virtualenv (python is only needed for generating the benchmarks and the plots)

### Tests

```make test``` executes all available test (~400 test cases that test the correctness of our implementation (no false-negatives) and different aspects like multi-threading, partitioning, vectorization, etc)

### Benchmarks

```make benchmark``` generates the benchmark and the user can select the benchmark to execute. The results are written to csv-files in the ```benchmark/paper``` folder. All measurements are repeated five times to get stable results.

Executing all benchmarks takes roughly 1 week and requires 64 GiB memory. Some of the benchmarks do measure only the false-positive rate and the failed builds and, thus, should be executed with all available threads.

The csv includes the following fields:
| Field         | Unit         | Description                                                  |
| ------------- | ------------ | ------------------------------------------------------------ |
| name          | Text         | Configuration: `<BenchmarkName>_<k>`/  `<Fixture>`/  `<s>` / `<n_threads>` / `<n_partitions>` / `<elements_build>` / `<elements_lookup>` / `<shared_elements>` / `_` / `_` |
| real_time     | milliseconds | execution time per iteration                                 |
| DTLB-misses   | float        | data translation lookaside buffer misses per iteration       |
| ITLB-misses   | float        | instruction translation lookaside buffer misses per iteration |
| L1D-misses    | float        | level 1 data cache misses per iteration                      |
| L1I-misses    | float        | level 1 instruction cache misses per iteration               |
| LLC-misses    | float        | last-level (L3) cache misses per iteration                   |
| branch-misses | float        | branch misses per iteration                                  |
| cycles        | float        | cycles per iteration                                         |
| instruction   | float        | executed instructions per iteration                          |
| FPR           | float        | false-positive rate of the filter (only available when lookup is benchmarked) |
| failures      | integer      | number of failed builds                                      |
| retries       | integer      | number of retries needed to build the filter                 |
| bits          | float        | number of bits per key allocated to the filter               |
| size          | integer      | size of the filter in bytes                                  |

### Plots

```make plot``` generates all plots used in the paper. The benchmark data is taken from `benchmark/paper`.

### Repository Structure

* `benchmark`: code for benchmarking the filter and the definition files with our results on the Skylake-X machine.
* `cmake`: optional packages.
* `lib`: external dependencies and existing filter implementations. *The code in this folder is not licensed under the MIT License (see Dependencies).*
* `python`: scripts for generating, executing and plotting benchmarks.
* `src`: filter implementations.
* `test`: extensive test cases for our filter implementations and the integration the competitors.
* `vendor`: external packages.

### Dependencies

* `lib/amd_mortonfilter`: original [Morton Filter](https://github.com/AMDComputeLibraries/morton_filter) implementation used in [3], licensed under **the MIT License**.
* `lib/bsd`: [(register-)blocked and (cache-)sectorized Bloom Filter](https://github.com/peterboncz/bloomfilter-bsd) implementations with SIMD support and external competitors used in [1], licensed under **the Apache License (Version 2.0), the 2-clause BSD License, and the 3-clause BSD License**.
* `lib/cityhash`: [Google's CityHash](https://github.com/google/cityhash) implementation, licensed under **the MIT License**.
* `lib/efficient_cuckoofilter`: original [Cuckoo Filter](https://github.com/efficient/cuckoofilter) implementation used in [2], licensed under **the Apache License (Version 2.0)**.
* `lib/fastfilter`: original [Xor Filter](https://github.com/FastFilter/fastfilter_cpp) implementation used in [4] licensed under **the Apache License (Version 2.0)**.
* `lib/impala`: original [sectorized Bloom Filter](https://github.com/apache/impala) used in the Impala, licensed under **the Apache License (Version 2.0)**.
* `lib/libdivide`: the [LibDivide](https://github.com/ridiculousfish/libdivide) library computes magic numbers for optimizing integer divisions, licensed under **the zlib License**.
* `lib/perfevent`: library for reading perf counters in C(++), licensed under **the MIT License**.
* `lib/vacuumfilter`: [Vacuum Filter](https://github.com/wuwuz/Vacuum-Filter) implementation.

## Related Work

[1] [Performance-Optimal Filtering: Bloom Overtakes Cuckoo at High Throughput](http://www.vldb.org/pvldb/vol12/p502-lang.pdf)

[2] [Cuckoo Filter: Practically Better Than Bloom](http://www.cs.cmu.edu/~binfan/papers/conext14_cuckoofilter.pdf)

[3] [Morton Filters: Faster, Space-Efficient Cuckoo Filters via Biasing, Compression, and Decoupled Logical Sparsity](https://www.vldb.org/pvldb/vol11/p1041-breslow.pdf)

[4] [Xor Filters: Faster and Smaller Than Bloom and Cuckoo Filters](https://arxiv.org/pdf/1912.08258.pdf)
