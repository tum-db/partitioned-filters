#pragma once

#include <iostream>
#include <memory>
#include <simd/helper.hpp>
#include <simd/vector.hpp>
#include <task/task_queue.hpp>
#include "filter_base.hpp"
#include "reference/bsd/bloom/implementation/bsd_bloom_implementation.hpp"

namespace filters {

    template<typename FP, size_t k, typename OP>
    struct Filter<FilterType::BSDBloom, FP, k, OP> {

        static constexpr bool supports_add = true;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::registerSize == parameter::RegisterSize::_32bit, "only 32bit integer are supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(
                OP::hashingMode == parameter::HashingMode::Mul or OP::hashingMode == parameter::HashingMode::Murmur,
                "only Mul and Murmur are supported!");
        static_assert(OP::addressingMode == parameter::AddressingMode::Magic ||
                      OP::addressingMode == parameter::AddressingMode::PowerOfTwo,
                "only Magic or PowerOfTwo Addressing is supported!");

        using I = bsd::bloom::Implementation<FP::variant, k, FP::word_cnt_per_block, FP::sector_zone_cnt,
                OP::addressingMode, OP::simd, OP::hashingMode, FP::batching>;

        const size_t n_partitions = 1;
        size_t s;
        std::unique_ptr<I> impl;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t, size_t n_threads, size_t n_tasks_per_level) : s(s), n_partitions(n_partitions),
                                                                               queue(n_threads, n_tasks_per_level) {
        }

        forceinline
        void init(const uint32_t *histogram) {
            impl = std::make_unique<I>(*histogram * k * s / 100);
        }

        forceinline
        bool contains(const uint32_t &value, const size_t = 0) {
            return impl->contains(value);
        }

        forceinline
        bool add(const uint32_t &value, const size_t = 0) {
            impl->add(value);
            return true;
        }

        bool construct(uint32_t *values, size_t length) {
            uint32_t histogram = length;
            init(&histogram);
            impl->insert(values, length);
            return true;
        }

        size_t count(uint32_t *values, size_t length) {
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                return impl->count(values, length);
            } else {
                std::atomic<size_t> counter{0};

                size_t begin = 0;
                for (size_t i = queue.get_n_tasks_per_level(); i > 0; i--) {
                    const size_t end = std::min(length - begin,
                            simd::valign<simd::Vector<OP::registerSize, OP::simd>>((length - begin) / i));

                    queue.add_task([this, &counter, values, begin, end](size_t) {
                        counter += impl->count(values + begin, end);
                    });
                    begin += end;
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() {
            return impl->size();
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
