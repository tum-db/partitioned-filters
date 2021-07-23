#pragma once

#include <filter_base.hpp>
#include <memory>
#include <amd_mortonfilter/morton_filter.h>

namespace filters {

    namespace mortonfilter = CompressedCuckoo;

    template<typename FP, size_t k, typename OP>
    struct Filter<FilterType::AMDMortonFilter, FP, k, OP> {

        static constexpr bool supports_add = true;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::simd == parameter::SIMD::Scalar, "only Scalar is supported!");
        static_assert(OP::registerSize == parameter::RegisterSize::_64bit, "only 64bit registers are supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(OP::hashingMode == parameter::HashingMode::Murmur, "only Murmur is supported!");
        static_assert(OP::addressingMode == parameter::AddressingMode::Lemire, "only Lemire is supported!");

        static constexpr size_t slots_per_bucket = static_cast<std::size_t>(FP::associativity);
        // slots_per_bucket == 1 -> k in {8}
        static_assert(slots_per_bucket != 1 or k == 8, "wrong k!");
        // slots_per_bucket == 3 -> k in {6, 8, 12, 16, 18}
        static_assert(slots_per_bucket != 3 or k == 6 or k == 8 or k == 12 or k == 16 or k == 18, "wrong k!");
        // slots_per_bucket == 7 -> k in {6, 8, 12, 16, 18}
        static_assert(slots_per_bucket != 7 or k == 6 or k == 8 or k == 12 or k == 16 or k == 18, "wrong k!");
        // slots_per_bucket == 15 -> k in {6, 8, 12, 16, 18}
        static_assert(slots_per_bucket != 15 or k == 6 or k == 8 or k == 12 or k == 16 or k == 18, "wrong k!");

        using Morton1 = mortonfilter::Morton1_8;
        using Morton3 = std::conditional_t<k == 6, mortonfilter::Morton3_6,
                std::conditional_t<k == 8, mortonfilter::Morton3_8,
                        std::conditional_t<k == 12, mortonfilter::Morton3_12,
                                std::conditional_t<k == 16, mortonfilter::Morton3_16, mortonfilter::Morton3_18>>>>;
        using Morton7 = std::conditional_t<k == 6, mortonfilter::Morton7_6,
                std::conditional_t<k == 8, mortonfilter::Morton7_8,
                        std::conditional_t<k == 12, mortonfilter::Morton7_12,
                                std::conditional_t<k == 16, mortonfilter::Morton7_16, mortonfilter::Morton7_18>>>>;
        using Morton15 = std::conditional_t<k == 6, mortonfilter::Morton15_6,
                std::conditional_t<k == 8, mortonfilter::Morton15_8,
                        std::conditional_t<k == 12, mortonfilter::Morton15_12,
                                std::conditional_t<k == 16, mortonfilter::Morton15_16, mortonfilter::Morton15_18>>>>;

        using T = uint64_t;
        using F = std::conditional_t<slots_per_bucket == 1, Morton1, std::conditional_t<slots_per_bucket == 3, Morton3,
                std::conditional_t<slots_per_bucket == 7, Morton7, Morton15>>>;

        size_t n_partitions = 1;
        size_t s;
        std::unique_ptr<F> filter;
        task::TaskQueue <OP::multiThreading> queue;

        Filter(size_t s, size_t, size_t n_threads, size_t n_tasks_per_level) : s(s), n_partitions(n_partitions),
                                                                               queue(n_threads, n_tasks_per_level) {
        }

        forceinline
        void init(const T *histogram) {
            filter = std::make_unique<F>(*histogram * s / 100);
        }

        forceinline
        bool contains(const T &value, const size_t = 0) {
            return filter->likely_contains(value);
        }

        forceinline
        bool add(const T &value, const size_t = 0) {
            return filter->insert(value);
        }

        bool construct(T *values, size_t length) {
            T histogram = length;
            init(&histogram);
            bool success = true;

            // TODO: check if batched is faster
            for (size_t i = 0; i < length; i++) {
                success &= add(values[i]);
            }
            return success;
        }

        size_t count(T *values, size_t length) {
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                size_t counter = 0;
                // TODO: check if batched is faster
                for (size_t i = 0; i < length; i++) {
                    counter += contains(values[i]);
                }
                return counter;
            } else {
                std::atomic<size_t> counter{0};

                size_t begin = 0;
                for (size_t i = queue.get_n_tasks_per_level(); i > 0; i--) {
                    const size_t end = begin + (length - begin) / i;
                    queue.add_task([this, &counter, values, begin, end](size_t) {
                        size_t local_counter = 0;
                        for (size_t i = begin; i < end; i++) {
                            local_counter += contains(values[i]);
                        }
                        counter += local_counter;
                    });
                    begin = end;
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() {
            return sizeof(typename F::block_t) * filter->_total_blocks;
        }

        size_t avg_size() {
            // no partitioning available
            return size();
        }

        size_t retries() {
            return 0;
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(k) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"filter_params\": " + FP::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters
