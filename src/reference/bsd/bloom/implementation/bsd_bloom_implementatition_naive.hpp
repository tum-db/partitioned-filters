#pragma once

#include <bsd/src/dtl/filter/bloomfilter/bloomfilter_logic.hpp>
#include <parameter/parameter.hpp>
#include "reference/bsd/bloom/bsd_bloom_parameter.hpp"
#include "bsd_bloom_implementation_base.hpp"

namespace filters::bsd::bloom {

    template<size_t k, size_t word_cnt_per_block, size_t sector_zone_cnt, parameter::AddressingMode addressing, parameter::SIMD simd, parameter::HashingMode hashing, Batching batching>
    struct Implementation<Variant::Naive, k, word_cnt_per_block, sector_zone_cnt, addressing, simd, hashing, batching> {

        // Library function contains_vec does not compile
        static_assert(simd == parameter::SIMD::Scalar, "BSD Bloom Filter Naive does not AVX!");

        // Library does not implement Murmur for Naive Bloom Filter
        static_assert(hashing == parameter::HashingMode::Mul);

        // Library tries to shift a 32 bit value left by a factor that is larger than 32 when using Magic Addressing.
        static_assert(addressing == parameter::AddressingMode::PowerOfTwo,
                "BSD Bloom Filter Naive does not support Magic Addressing!");
        static constexpr dtl::block_addressing addr = dtl::block_addressing::POWER_OF_TWO;

        using T = uint32_t;
        static constexpr size_t simd_lanes = static_cast<size_t>(simd) / sizeof(T) / 8;
        static_assert(simd == parameter::SIMD::Scalar or dtl::simd::lane_count<key_t> == simd_lanes,
                "Wrong SIMD configuration!");

        using filter_t = dtl::bloomfilter_logic<T, addr>;

        dtl::mem::allocator_config alloc_config = dtl::mem::allocator_config::local();
        dtl::mem::numa_allocator<T> allocator{alloc_config};

        const filter_t filter;
        std::vector<T, dtl::mem::numa_allocator<T>> filter_data;

        explicit Implementation(size_t size_in_bits) : filter(size_in_bits, k),
                                                       filter_data(filter.size(), 0, allocator) {
        }

        forceinline
        void add(const T &value) {
            filter.insert(&filter_data[0], value);
        }

        forceinline
        bool contains(const T &value) const {
            return filter.contains(&filter_data[0], value);
        }

        forceinline
        void insert(T *values, T length) {
            filter.batch_insert(&filter_data[0], values, length);
        }

        forceinline
        size_t count(const T *values, T length) const {
            size_t counter = 0;
            if constexpr (batching == Batching::Disabled) {
                for (size_t i = 0; i < length; i++) {
                    counter += contains(values[i]);
                }
            } else {
                auto *match_positions = new uint32_t[length];
                counter = filter.batch_contains(&filter_data[0], values, length, match_positions, 0);
                delete[] match_positions;
            }
            return counter;
        }

        forceinline
        size_t size() const {
            return filter.get_length() / 8;
        }

    };
}