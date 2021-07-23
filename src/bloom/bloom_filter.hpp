#pragma once

#include <iostream>
#include <bloom/container/bloom_container.hpp>
#include <bloom/construct/bloom_construct.hpp>
#include <bloom/count/bloom_count.hpp>
#include <filter_base.hpp>
#include <address/addresser.hpp>
#include <task/task_queue.hpp>

namespace filters {

    template<typename FilterParameter, size_t _k, typename OptimizationParameter>
    struct Filter<FilterType::Bloom, FilterParameter, _k, OptimizationParameter> {

        using FP = FilterParameter;
        static constexpr size_t k = _k;
        using OP = OptimizationParameter;
        static constexpr bool supports_add = OP::simd == parameter::SIMD::Scalar or
                                             (FP::variant == bloom::Variant::Sectorized and
                                              FP::vectorization == bloom::Vectorization::Vertical);
        static constexpr bool supports_add_partition = supports_add;

        // (variant == Sectorized and blockSize == 32bit) -> registerSize == 32bit
        static_assert(not(FP::variant == bloom::Variant::Sectorized and FP::blockSize == bloom::BlockSize::_32bit) or
                      OP::registerSize == parameter::RegisterSize::_32bit,
                "Register Blocked variants expects the corresponding Register Size!");
        // (variant == Sectorized and blockSize == 64bit) -> registerSize == 64bit
        static_assert(not(FP::variant == bloom::Variant::Sectorized and FP::blockSize == bloom::BlockSize::_64bit) or
                      OP::registerSize == parameter::RegisterSize::_64bit,
                "Register Blocked variants expects the corresponding Register Size!");

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;
        using Addresser = addresser::Addresser<OP::addressingMode, Vector>;
        using Container = bloom::BloomContainer<FP::blockSize, OP::partitioning, Vector, Addresser>;
        using Hasher = hash::Hasher<OP::hashingMode, Vector, 0>;
        using PartitionSet = partition::PartitionSet<OP::partitioning, Vector>;

        using Constructor = bloom::BloomConstructor<FP::variant, FP::vectorization, FP::blockSize, FP::groups, k,
                OP::registerSize, OP::simd, Container, Hasher>;
        using Counter = bloom::BloomCounter<FP::variant, FP::vectorization, FP::blockSize, FP::groups, k,
                OP::registerSize, OP::simd, Container, Hasher>;

        size_t s, n_partitions;
        Container container;
        Constructor constructor;
        Counter counter;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t n_partitions, size_t n_threads, size_t n_tasks_per_level) : s(s),
                                                                                            n_partitions(n_partitions),
                                                                                            queue(n_threads,
                                                                                                    n_tasks_per_level) {
        }

        forceinline
        void init(const T *histogram) {
            static_assert(supports_add_partition, "not supported!");

            n_partitions = std::max(n_partitions, 1ul);
            container = std::move(Container(k, s, n_partitions, histogram));
        }

        forceinline
        bool contains(const T &value) const {
            return contains(value, value & (n_partitions - 1));
        }

        forceinline
        bool contains(const T &value, size_t index) const {
            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                return counter.contains(container, Vector(value), 1, 0);
            } else {
                return counter.contains(container, Vector(value), 1, index);
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
                constructor.construct_partition(container, values, length, 0);
            } else {
                constructor.construct_partition(container, values, length, index);
            }
            return true;
        }

        forceinline
        bool add(const T &value) {
            return add(value, value & (n_partitions - 1));
        }

        forceinline
        bool add(const T &value, size_t index) {
            static_assert(supports_add, "not supported!");

            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                constructor.insert(container, Vector(value), 1, 0);
            } else {
                constructor.insert(container, Vector(value), 1, index);
            }
            return true;
        }

        bool construct(T *values, size_t length) {
            PartitionSet partitions(n_partitions);
            n_partitions = partitions.n_partitions;

            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                partitions.init(values, length);
                container = std::move(Container(k, s, n_partitions, partitions.histogram));
                constructor.construct(container, partitions);
            } else {
                partitions.init(values, length, queue);

                queue.add_task([&](size_t) {
                    container = std::move(Container(k, s, n_partitions, partitions.histogram));
                });
                queue.add_barrier();

                constructor.construct(container, partitions, queue);
                queue.execute_tasks();
            }
            return true;
        }

        size_t count(T *values, size_t length) {
            PartitionSet partitions(n_partitions);

            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                partitions.init(values, length);

                return counter.count(container, partitions);
            } else {
                partitions.init(values, length, queue);

                std::atomic<size_t> count{0};
                counter.count(container, partitions, count, queue);
                queue.execute_tasks();

                return count;
            }
        }

        size_t size() const {
            return container.length();
        }

        size_t avg_size() const {
            return size() / n_partitions;
        }

        size_t retries() const {
            return 0;
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(k) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"avg_size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"n_partitions\": " + std::to_string(n_partitions) + ",\n";
            s += "\t\"filter_params\": " + FP::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters