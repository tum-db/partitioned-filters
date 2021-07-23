#pragma once

#include <iostream>
#include <filter_base.hpp>
#include <xorfilter/xorfilter_singleheader.h>
#include "fastfilter_xor_parameter.hpp"

namespace filters {

    template<typename OP>
    struct Filter<FilterType::FastfilterXor, fastfilter::_xor::CImplementation<8>, 8, OP> {

        static constexpr bool supports_add = false;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::simd == parameter::SIMD::Scalar, "only Scalar is supported!");
        static_assert(OP::registerSize == parameter::RegisterSize::_64bit, "only 64 bit registers are supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(OP::hashingMode == parameter::HashingMode::Murmur, "only Murmur is supported!");
        static_assert(OP::addressingMode == parameter::AddressingMode::Lemire, "only Lemire is supported!");

        const size_t n_partitions = 1;
        xor8_s filter;
        task::TaskQueue <OP::multiThreading> queue;

        Filter(size_t, size_t, size_t n_threads, size_t n_tasks_per_level) : n_partitions(n_partitions),
                                                                             queue(n_threads, n_tasks_per_level) {
            filter.fingerprints = nullptr;
        }

        ~Filter() {
            if (filter.fingerprints) {
                xor8_free(&filter);
                filter.fingerprints = nullptr;
            }
        }

        forceinline
        void init(const uint64_t *histogram) {
            if (filter.fingerprints) {
                xor8_free(&filter);
                filter.fingerprints = nullptr;
            }
            if (!xor8_allocate(*histogram, &filter)) {
                throw ::std::runtime_error("allocation failed");
            }
        }

        forceinline
        bool contains(const uint64_t &value, const size_t = 0) {
            return xor8_contain(value, &filter);
        }

        bool construct(uint64_t *values, size_t length) {
            init(&length);
            return xor8_populate(values, length, &filter);
        }

        size_t count(uint64_t *values, size_t length) {
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                size_t counter = 0;
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
            return xor8_size_in_bytes(&filter);
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
            s += "\t\"filter_params\": " + fastfilter::_xor::CImplementation<8>::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

    template<typename OP>
    struct Filter<FilterType::FastfilterXor, fastfilter::_xor::CImplementation<16>, 16, OP> {

        static constexpr bool supports_add = false;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::simd == parameter::SIMD::Scalar, "only Scalar is supported!");
        static_assert(OP::registerSize == parameter::RegisterSize::_64bit, "only 64 bit registers are supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(OP::hashingMode == parameter::HashingMode::Murmur, "only Murmur is supported!");
        static_assert(OP::addressingMode == parameter::AddressingMode::Lemire, "only Lemire is supported!");

        const size_t n_partitions = 1;
        xor16_t filter;
        task::TaskQueue <OP::multiThreading> queue;

        Filter(size_t, size_t, size_t n_threads, size_t n_tasks_per_level) : n_partitions(n_partitions),
                                                                             queue(n_threads, n_tasks_per_level) {
            filter.fingerprints = nullptr;
        }

        ~Filter() {
            if (filter.fingerprints) {
                xor16_free(&filter);
                filter.fingerprints = nullptr;
            }
        }

        forceinline
        void init(const uint64_t *histogram) {
            if (filter.fingerprints) {
                xor16_free(&filter);
                filter.fingerprints = nullptr;
            }
            if (!xor16_allocate(*histogram, &filter)) {
                throw ::std::runtime_error("allocation failed");
            }
        }

        forceinline
        bool contains(const uint64_t &value) {
            return xor16_contain(value, &filter);
        }

        bool construct(uint64_t *values, size_t length) {
            init(&length);
            return xor16_populate(values, length, &filter);
        }

        size_t count(uint64_t *values, size_t length) {
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                size_t counter = 0;
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
            return xor16_size_in_bytes(&filter);
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(8) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"filter_params\": " + fastfilter::_xor::CImplementation<16>::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters
