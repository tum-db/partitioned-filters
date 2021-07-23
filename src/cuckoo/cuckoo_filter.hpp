#pragma once

#include <iostream>
#include <bloom/container/bloom_container.hpp>
#include <bloom/construct/bloom_construct.hpp>
#include <bloom/count/bloom_count.hpp>
#include <filter_base.hpp>
#include <address/addresser.hpp>
#include <cuckoo/container/cuckoo_container.hpp>

namespace filters {

    template<typename FilterParameter, size_t _k, typename OptimizationParameter>
    struct Filter<FilterType::Cuckoo, FilterParameter, _k, OptimizationParameter> {

        using FP = FilterParameter;
        static constexpr size_t k = _k;
        using OP = OptimizationParameter;
        static constexpr bool retry = OP::partitioning == parameter::Partitioning::Enabled or
                                      (FP::variant == cuckoo::Variant::Morton and OP::simd != parameter::SIMD::Scalar);
        static constexpr bool supports_add = OP::simd == parameter::SIMD::Scalar and not retry;
        static constexpr bool supports_add_partition = supports_add;

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;
        using Container = cuckoo::CuckooContainer<retry, FP::variant, FP::associativity, FP::buckets_per_block,
                FP::ota_size, k, OP::partitioning, OP::registerSize, OP::simd, OP::addressingMode, OP::hashingMode>;
        using PartitionSet = typename partition::PartitionSet<OP::partitioning, Vector>;

        size_t s;
        size_t n_partitions;
        Container container;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t n_partitions, size_t n_threads, size_t n_tasks_per_level) : s(s),
                                                                                            n_partitions(n_partitions),
                                                                                            queue(n_threads,
                                                                                                    n_tasks_per_level) {
        }

        forceinline
        void init(const T *histogram) {
            static_assert(supports_add, "not supported!");

            n_partitions = std::max(n_partitions, 1ul);
            container = std::move(Container(s, n_partitions, histogram));
        }

        forceinline
        bool contains(const T &value) const {
            return contains(value, value & (n_partitions - 1));
        }

        forceinline
        bool contains(const T &value, size_t index) const {
            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                return container.contains(Vector(value), 0, 1);
            } else {
                return container.contains(Vector(value), index, 1);
            }
        }

        forceinline
        bool add_all(const T *values, size_t length) {
            return add_partition(values, length, 0);
        }

        forceinline
        bool add_partition(const T *values, size_t length, size_t index) {
            static_assert(supports_add, "not supported!");

            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                return container.construct(Vector(0), Vector(length), values, 0);
            } else {
                return container.construct(Vector(0), Vector(length), values, index);
            }
        }

        forceinline
        bool add(const T &value) {
            return add(value, value & (n_partitions - 1));
        }

        forceinline
        bool add(const T &value, size_t index) {
            static_assert(supports_add, "not supported!");

            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                return container.insert(Vector(value), 0, 1);
            } else {
                return container.insert(Vector(value), index, 1);
            }
        }

        bool construct(T *values, size_t length) {
            PartitionSet partitions(n_partitions);
            n_partitions = partitions.n_partitions;

            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                partitions.init(values, length);
                container = std::move(Container(s, n_partitions, partitions.histogram));

                bool success = true;
                for (size_t i = 0; i < partitions.n_partitions and success; i += Vector::n_elements) {
                    const Vector begin = Vector::load(partitions.offsets + i);
                    const Vector end = begin + Vector::load(partitions.histogram + i);
                    success &= (Vector::mask(1) == container.construct(begin, end, partitions.values, i));
                }
                return success;
            } else {
                partitions.init(values, length, queue);

                queue.add_task([&](size_t) {
                    container = std::move(Container(s, n_partitions, partitions.histogram));
                });
                queue.add_barrier();

                std::atomic<uint8_t> success{1};
                for (size_t i = 0; i < partitions.n_partitions; i += Vector::n_elements) {
                    queue.add_task([this, &success, &partitions, i](size_t) {
                        const Vector begin = Vector::load(partitions.offsets + i);
                        const Vector end = begin + Vector::load(partitions.histogram + i);
                        success &= (Vector::mask(1) == container.construct(begin, end, partitions.values, i));
                    });
                }
                queue.execute_tasks();

                return success;
            }
        }

        forceinline
        size_t _count(const T *begin, const T *end, size_t i) const {
            size_t counter = 0;
            for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                const Vector values = Vector::load(begin);
                counter += Vector::popcount_mask(container.contains(values, i, Vector::mask(1)));
            }

            if constexpr (Vector::avx) {
                if (begin < end) {
                    M mask = (1 << (end - begin)) - 1;
                    const Vector values = Vector::load(begin);
                    counter += Vector::popcount_mask(container.contains(values, i, mask));
                }
            }

            return counter;
        }

        size_t count(T *values, size_t length) {
            PartitionSet partitions(n_partitions);
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                partitions.init(values, length);
                size_t counter = 0;

                for (size_t i = 0; i < partitions.n_partitions; i += 1) {
                    const T *begin = partitions.values + partitions.offsets[i];
                    const T *end = begin + partitions.histogram[i];

                    counter += _count(begin, end, i);
                }

                return counter;
            } else {
                partitions.init(values, length, queue);

                std::atomic<size_t> counter{0};
                size_t remaining_tasks = queue.get_n_tasks_per_level();
                for (size_t i = 0; i < partitions.n_partitions; i++) {
                    // add at least one task
                    size_t n_tasks = 1;
                    if (remaining_tasks > 0) {
                        n_tasks = std::max(remaining_tasks / (partitions.n_partitions - i), static_cast<size_t>(1));
                        remaining_tasks -= n_tasks;
                    }

                    for (size_t j = 0; j < n_tasks; j++) {
                        queue.add_task([this, &counter, &partitions, i, j, n_tasks](size_t) {
                            const size_t length = partitions.histogram[i];
                            const T *begin = partitions.values + partitions.offsets[i] +
                                             simd::valign<Vector>((length / n_tasks + 1) * j);
                            const T *end = partitions.values + partitions.offsets[i] +
                                           std::min(simd::valign<Vector>((length / n_tasks + 1) * (j + 1)), length);

                            counter += _count(begin, end, i);
                        });
                    }
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() const {
            return container.length();
        }

        size_t avg_size() const {
            return size() / n_partitions;
        }

        size_t retries() const {
            return container.retries();
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(k) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"avg_size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"n_partitions\": " + std::to_string(n_partitions) + ",\n";
            s += "\t\"retries\": " + std::to_string(retries()) + ",\n";
            s += "\t\"filter_params\": " + FP::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters