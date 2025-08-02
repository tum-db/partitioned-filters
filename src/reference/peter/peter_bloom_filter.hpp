#pragma once

#include <atomic>
#include "filter_base.hpp"
#include <simd/vector.hpp>
#include <address/addresser.hpp>
#include <reference/peter/peter_bloom_filter.hpp>
#include <hash/hasher.hpp>
#include "peter_bloom_parameter.hpp"
#include <vector>
#include <cmath>
#include <iostream>
#include <task/task_queue.hpp>

#define BF_RESTRICT __restrict__

namespace filters {
    template <parameter::HashingMode hashingMode>
    uint32_t containsMany(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);

    template <typename OP>
    struct Filter<FilterType::PeterBloom, peter::Standard<7>, 7, OP> {
        static constexpr bool supports_add = false;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::simd == parameter::SIMD::AVX512 or OP::simd == parameter::SIMD::Scalar, "only AVX512 is supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(OP::addressingMode == parameter::AddressingMode::PowerOfTwo, "address must be power of 2!");
        static_assert(OP::registerSize == parameter::RegisterSize::_64bit, "registerSize must be 64bit aligned!");

        using Scalar = simd::Vector<OP::registerSize, parameter::SIMD::Scalar>;
        using T = typename Scalar::T;
        using Hasher = hash::Hasher<OP::hashingMode, Scalar, 0>;

        const size_t n_partitions = 1;
        const size_t k = 7;

        const uint32_t MAX_NUM_BLOCKS = (1 << 26);


        size_t s;
        uint32_t num_blocks;
        uint32_t num_blocks_log;
        uint32_t* blocks = nullptr;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t, size_t n_threads, size_t n_tasks_per_level)
            : s(s), n_partitions(n_partitions), queue(n_threads, n_tasks_per_level) {}

        ~Filter() {
            if (blocks) {
                free(blocks);
            }
        }

        forceinline

        void init(const T* histogram) {
            num_blocks = *histogram * k * s / 100 / (sizeof(uint32_t) * 8) + 1;
            num_blocks_log = static_cast<uint32_t>(std::log2(num_blocks)) + 1;
            num_blocks = std::min(1U << num_blocks_log, MAX_NUM_BLOCKS);

            blocks = static_cast<uint32_t*>(aligned_alloc(64, sizeof(uint32_t) * num_blocks));
            std::memset(blocks, 0, sizeof(uint32_t) * num_blocks);
        }

        forceinline bool contains(const T& value, const size_t = 0) const {
            auto*BF_RESTRICT bf = blocks;
            const uint64_t h = Hasher::hash(Scalar(value)).vector;
            uint32_t key_lo = static_cast<uint32_t>(h);
            uint32_t key_hi = h >> 32;
            uint32_t block1 = ((key_lo & ((1 << 17) - 1)) + // key_lo 13:block|4:sector1
                    ((key_hi << 14) & (((1 << 9) - 1) << 17))) // key_hi 9:block
                & (num_blocks - 1);
            uint32_t mask1 = (1 << ((key_lo >> 17) & 31)) // key_lo 5:bit1
                | (1 << ((key_lo >> 22) & 31)) // key_lo 5:bit2
                | (1 << ((key_lo >> 27) & 31)); // key_lo 5:bit3
            uint32_t block2 = block1 ^ (8 + (key_hi & 7)); // key_hi 3:sector2
            uint32_t mask2 = (1 << ((key_hi >> 12) & 31)) // key_hi 5:bit1
                | (1 << ((key_hi >> 17) & 31)) // key_hi 5:bit2
                | (1 << ((key_hi >> 22) & 31)) // key_hi 5:bit3
                | (1 << ((key_hi >> 27) & 31)); // key_hi 5:bit4
            return ((bf[block2] & mask2) == mask2) & ((bf[block1] & mask1) == mask1);
        }


        forceinline bool add(const T& value, const size_t = 0) {
            auto* bf = blocks;
            const T h = Hasher::hash(Scalar(value)).vector;
            int32_t key_lo = reinterpret_cast<const uint32_t* BF_RESTRICT>(&h)[0];
            uint32_t key_hi = reinterpret_cast<const uint32_t* BF_RESTRICT>(&h)[1];
            uint32_t block1 = ((key_lo & ((1 << 17) - 1)) + // key_lo 13:block|4:sector1
                    ((key_hi << 14) & (((1 << 9) - 1) << 17))) // key_hi 9:block
                & (num_blocks - 1);
            uint32_t mask1 = (1 << ((key_lo >> 17) & 31)) // key_lo 5:bit1
                | (1 << ((key_lo >> 22) & 31)) // key_lo 5:bit2
                | (1 << ((key_lo >> 27) & 31)); // key_lo 5:bit3
            uint32_t block2 = block1 ^ (8 + (key_hi & 7)); // key_hi 3:sector2
            uint32_t mask2 = (1 << ((key_hi >> 12) & 31)) // key_hi 5:bit1
                | (1 << ((key_hi >> 17) & 31)) // key_hi 5:bit2
                | (1 << ((key_hi >> 22) & 31)) // key_hi 5:bit3
                | (1 << ((key_hi >> 27) & 31)); // key_hi 5:bit4
            bf[block1] |= mask1;
            bf[block2] |= mask2;
            return true;
        }

        bool construct(T* values, size_t length) {
            T histogram = length;
            init(&histogram);
            for (size_t i = 0; i < length; i++) {
                add(values[i]);
            }
            return true;
        }

        size_t count(T* values, size_t length) {
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                if constexpr (OP::simd == parameter::SIMD::Scalar) {
                    size_t counter = 0;
                    for (size_t i = 0; i < length; i++) {
                        counter += contains(values[i]);
                    }
                    return counter;
                }
                else {
                    return containsMany<OP::hashingMode>(length, num_blocks, values, blocks);
                }
            }
            else {
                std::atomic<size_t> counter{0};

                size_t begin = 0;
                for (size_t i = queue.get_n_tasks_per_level(); i > 0; i--) {
                    const size_t end = begin + (length - begin) / i;
                    queue.add_task([this, &counter, values, begin, end](size_t) {
                        if constexpr (OP::simd == parameter::SIMD::Scalar) {
                            size_t local_counter = 0;
                            for (size_t i = begin; i < end; i++) {
                                local_counter += contains(values[i]);
                            }
                            counter += local_counter;
                        }
                        else {
                            counter += containsMany<OP::hashingMode>(end - begin, num_blocks, &values[begin], blocks);
                        }
                    });
                    begin = end;
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() {
            return num_blocks * sizeof(uint32_t);
        }

        size_t avg_size() {
            // no partitioning available
            return size();
        }

        size_t retries() {
            // cannot get the number of retries needed for building from the implementation
            return 0;
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(8) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"filter_params\": " + peter::Standard<8>::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };
} // filters
