#pragma once

#include <filter_base.hpp>
#include <simd/vector.hpp>
#include <address/addresser.hpp>
#include <hash/hasher.hpp>
#include "umbra_bloom_parameter.hpp"
#include "task/task_queue.hpp"
#include <vector>
#include "join_filter.hpp"

#define BF_RESTRICT __restrict__

namespace filters {
    template <parameter::HashingMode hashingMode>
    uint32_t umbraContainsMany(int num, uint32_t shift, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);

    template <parameter::HashingMode hashingMode>
    uint32_t umbraContainsMany2(int num, uint32_t shift, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);

    template <typename OP>
    struct Filter<FilterType::UmbraBloom, umbra::Standard<4>, 4, OP> {
        static constexpr bool supports_add = false;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(OP::addressingMode == parameter::AddressingMode::PowerOfTwo, "address must be power of 2!");
        static_assert(OP::registerSize == parameter::RegisterSize::_64bit, "registerSize must be 64bit aligned!");

        using Scalar = simd::Vector<OP::registerSize, parameter::SIMD::Scalar>;
        using T = typename Scalar::T;
        using Hasher = hash::Hasher<OP::hashingMode, Scalar, 0>;

        const size_t n_partitions = 1;
        const size_t k = 4;

        size_t s;
        size_t shift, n_blocks;
        uint16_t* data = nullptr;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t, size_t n_threads, size_t n_tasks_per_level)
            : s(s), n_partitions(n_partitions), queue(n_threads, n_tasks_per_level) {}

        ~Filter() {
            if (data) {
                free(data);
            }
        }

        forceinline

        void init(const T* histogram) {
            n_blocks = *histogram * k * s / 100 / (sizeof(uint16_t) * 8) + 1;
            n_blocks = math::next_power_of_two(n_blocks);
            shift = 64 - math::get_number_of_bits(n_blocks);

            data = static_cast<uint16_t*>(aligned_alloc(64, sizeof(uint16_t) * n_blocks));
            std::memset(data, 0, sizeof(uint16_t) * n_blocks);
        }

        forceinline

        bool contains(const T& value, const size_t = 0) {
            const T h = Hasher::hash(Scalar(value)).vector;
            return JoinFilter::checkEntry(static_cast<uint32_t>(h), data[h >> shift]);
        }

        forceinline

        bool add(const T& value, const size_t = 0) {
            const T h = Hasher::hash(Scalar(value)).vector;
            data[h >> shift] |= JoinFilter::getMask(static_cast<uint32_t>(h));
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
                else if (OP::simd == parameter::SIMD::AVX512) {
                    return umbraContainsMany<OP::hashingMode>(length, shift, values, reinterpret_cast<uint32_t*>(data));
                }
                else {
                    return umbraContainsMany2<OP::hashingMode>(length, shift, values, reinterpret_cast<uint64_t*>(data));
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
                        else if (OP::simd == parameter::SIMD::AVX512) {
                            counter += umbraContainsMany<OP::hashingMode>(end - begin, shift, &values[begin], reinterpret_cast<uint32_t*>(data));
                        }
                        else {
                            counter += umbraContainsMany2<OP::hashingMode>(end - begin, shift, &values[begin], reinterpret_cast<uint64_t*>(data));
                        }
                    });
                    begin = end;
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() {
            return n_blocks * sizeof(uint16_t);
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
            s += "\t\"k\": " + std::to_string(4) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"filter_params\": " + umbra::Standard<4>::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };
} // filters
