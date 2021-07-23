#pragma once

#include <parameter/parameter.hpp>
#include <bsd/src/dtl/filter/bloomfilter/bloomfilter_logic.hpp>
#include <bsd/src/dtl/filter/blocked_bloomfilter/blocked_bloomfilter.hpp>
#include <bsd/src/dtl/filter/blocked_bloomfilter/blocked_bloomfilter_tune_impl.hpp>
#include "reference/bsd/bloom/bsd_bloom_parameter.hpp"

namespace filters::bsd::bloom {

    template<Variant variant, size_t k, size_t word_cnt_per_block, size_t sector_zone_cnt, parameter::AddressingMode addressing, parameter::SIMD simd, parameter::HashingMode hashing, Batching batching>
    struct Implementation {


        // simd == Scalar -> batching == Disabled
        static_assert(simd != parameter::SIMD::Scalar or batching == Batching::Disabled,
                "Batching not available in Scalar Mode!");

        template<typename key_t, uint32_t hash_fn_no> using hasher = typename std::conditional<
                hashing == parameter::HashingMode::Murmur, dtl::hasher_murmur<key_t, hash_fn_no>,
                dtl::hasher_mul<key_t, hash_fn_no>>::type;

        static constexpr dtl::block_addressing addr = (addressing == parameter::AddressingMode::PowerOfTwo)
                                                      ? dtl::block_addressing::POWER_OF_TWO
                                                      : dtl::block_addressing::MAGIC;

        // TODO: get early out as parameter
        static constexpr bool early_out = false;

        using T = uint32_t;
        static constexpr size_t simd_lanes = static_cast<size_t>(simd) / sizeof(T) / 8;
        // AVX enabled -> dtl::lane_count == simd_lanes
        static_assert(simd == parameter::SIMD::Scalar or dtl::simd::lane_count<T> == simd_lanes,
                "Wrong SIMD configuration!");

        using key_t = T;
        using hash_value_t = T;
        using word_t = T;
        // The first hash function to use inside the block. Note: 0 is used for block addressing
        static constexpr u32 block_hash_fn_idx = 1;
        using bbf_block_t = typename std::conditional<variant == Variant::Blocked,
                typename dtl::blocked_bloomfilter_block_logic<key_t, word_t, word_cnt_per_block, sector_zone_cnt, k,
                        hasher, hash_value_t, early_out, block_hash_fn_idx>::type,
                dtl::multizone_block<key_t, word_t, word_cnt_per_block, sector_zone_cnt, k, hasher, hash_value_t,
                        block_hash_fn_idx, 0, sector_zone_cnt, early_out>>::type;
        using bbf = dtl::blocked_bloomfilter_logic<key_t, hasher, bbf_block_t, addr, early_out>;

        dtl::mem::allocator_config alloc_config = dtl::mem::allocator_config::local();
        dtl::mem::numa_allocator<T> allocator{alloc_config};

        bbf filter;
        std::vector<T, dtl::mem::numa_allocator<T>> filter_data;

        explicit Implementation(size_t size_in_bits) : filter(size_in_bits),
                                                       filter_data(filter.word_cnt(), 0, allocator) {
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

            if constexpr (simd == parameter::SIMD::Scalar) {
                for (size_t i = 0; i < length; i++) {
                    counter += contains(values[i]);
                }
            } else {
                if constexpr (batching == Batching::Enabled) {
                    // 4 is the default configuration for loop unrolling, therefore we use it
                    static constexpr u64 vector_len = simd_lanes * 4;
                    auto *match_positions = new uint32_t[length];
                    counter = filter.template batch_contains<vector_len>(&filter_data[0], values, length,
                            match_positions, 0);
                    delete[] match_positions;
                } else {
                    using Tv = dtl::vec<key_t, simd_lanes>;
                    size_t i = 0;
                    for (; i + simd_lanes <= length; i += simd_lanes) {
                        typename Tv::mask m = filter.template contains_vec<simd_lanes>(&filter_data[0],
                                *reinterpret_cast<const Tv *>(values + i));
                        #ifdef __AVX512F__
                        counter += _mm_popcnt_u32(m.data.data);
                        #else
                        const uint32_t imask = _mm256_movemask_epi8(m.data.data);
                        counter += _mm_popcnt_u32(_pext_u32(imask, 0x11111111));
                        #endif
                    }
                    for (; i < length; i++) {
                        counter += filter.contains(&filter_data[0], values[i]);
                    }
                }
            }

            return counter;
        }

        forceinline
        size_t size() const {
            return filter.get_length() / 8;
        }

    };

}
