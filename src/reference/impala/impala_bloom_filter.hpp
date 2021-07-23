#pragma once

#include <filter_base.hpp>
#include <simd/vector.hpp>
#include <address/addresser.hpp>
#include <impala/block_bloom_filter.h>
#include <hash/hasher.hpp>
#include "impala_bloom_parameter.hpp"

namespace filters {

    template<typename OP>
    struct Filter<FilterType::ImpalaBloom, impala::Standard<8>, 8, OP> {

        static constexpr bool supports_add = true;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::simd == parameter::SIMD::AVX2, "only AVX2 is supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");

        using Scalar = simd::Vector<OP::registerSize, parameter::SIMD::Scalar>;
        using AVX2Vector = simd::Vector<OP::registerSize, parameter::SIMD::AVX2>;
        using T = typename Scalar::T;
        using Addresser = addresser::Addresser<OP::addressingMode, Scalar>;
        using Hasher = hash::Hasher<OP::hashingMode, Scalar, 0>;

        const size_t n_partitions = 1;

        size_t s;
        size_t n_bytes;
        Addresser addresser;
        kudu::BlockBloomFilter filter;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t, size_t n_threads, size_t n_tasks_per_level) : s(s), n_partitions(n_partitions),
                                                                               queue(n_threads, n_tasks_per_level) {
            filter.directory_ = nullptr;
        }

        ~Filter() {
            if (filter.directory_) {
                free(filter.directory_);
            }
        }

        forceinline
        void init(const T *histogram) {
            T n_blocks = *histogram * s / 100 / sizeof(AVX2Vector) + 1;
            addresser = std::move(Addresser(&n_blocks, 1));

            if (filter.directory_) {
                free(filter.directory_);
            }

            n_bytes = simd::valign<AVX2Vector>(addresser.get_size(0) * sizeof(AVX2Vector));
            filter.directory_ = reinterpret_cast<kudu::BlockBloomFilter::Bucket *>(simd::valloc<AVX2Vector>(n_bytes,
                    0));
        }

        forceinline
        bool contains(const T &value, const size_t = 0) {
            const Scalar v = Scalar(value);
            const Scalar h = Hasher::hash(Scalar(v));
            const Scalar bucket_idx = addresser.compute_address_vertical(0, simd::extractAddressBits(h));

            if constexpr (OP::registerSize == parameter::RegisterSize::_32bit) {
                return filter.BucketFindAVX2(bucket_idx.vector, Hasher::template rehash<0>(h, v).vector);
            } else {
                return filter.BucketFindAVX2(bucket_idx.vector, h.vector >> 32);
            }
        }

        forceinline
        bool add(const T &value, const size_t = 0) {
            const Scalar scalar = Scalar(value);
            const Scalar hash = Hasher::hash(Scalar(scalar));
            const Scalar bucket_idx = addresser.compute_address_vertical(0, simd::extractAddressBits(hash));

            if constexpr (OP::registerSize == parameter::RegisterSize::_32bit) {
                filter.BucketInsertAVX2(bucket_idx.vector, Hasher::template rehash<0>(hash, scalar).vector);
            } else {
                filter.BucketInsertAVX2(bucket_idx.vector, hash.vector >> 32);
            }

            return true;
        }

        bool construct(T *values, size_t length) {
            T histogram = length;
            init(&histogram);
            for (size_t i = 0; i < length; i++) {
                add(values[i]);
            }
            return true;
        }

        size_t count(T *values, size_t length) {
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
            return n_bytes;
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
            s += "\t\"filter_params\": " + impala::Standard<8>::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters
