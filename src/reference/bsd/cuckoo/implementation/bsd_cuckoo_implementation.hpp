#pragma once

#include <parameter/parameter.hpp>
#include <bsd/src/dtl/filter/cuckoofilter/cuckoofilter_logic.hpp>
#include <reference/bsd/cuckoo/bsd_cuckoo_parameter.hpp>

namespace filters::bsd::cuckoo {

    template<size_t k, size_t tags_per_bucket, parameter::AddressingMode addressing, parameter::SIMD simd, Batching batching>
    struct Implementation {

        static_assert(k == 4 or k == 8 or k == 12 or k == 16 or k == 32, "fingerprint size not supported!");

        // (simd == AVX2 or simd == AVX512) -> (k in {8, 16, 32})
        static_assert(simd == parameter::SIMD::Scalar or k == 8 or k == 16 or k == 32,
                "AVX requires fingerprints with power-of-two size!");

        // simd == Scalar -> batching == Disabled
        static_assert(simd != parameter::SIMD::Scalar or batching == Batching::Disabled,
                "Batching not available in Scalar Mode!");

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
        using cf = dtl::cuckoofilter::cuckoofilter_logic<k, tags_per_bucket, dtl::cuckoofilter::cuckoofilter_table,
                addr>;

        dtl::mem::allocator_config alloc_config = dtl::mem::allocator_config::local();
        dtl::mem::numa_allocator<T> allocator{alloc_config};

        cf filter;
        std::vector<T, dtl::mem::numa_allocator<T>> filter_data;

        explicit Implementation(size_t size_in_bits) : filter(size_in_bits),
                                                       filter_data(filter.word_cnt(), 0, allocator) {
        }

        forceinline
        bool add(const T &value) {
            return filter.insert(&filter_data[0], value);
        }

        forceinline
        bool contains(const T &value) const {
            return filter.contains(&filter_data[0], value);
        }

        forceinline
        bool insert(T *values, T length) {
            return filter.batch_insert(&filter_data[0], values, length);
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
                        typename Tv::mask m = filter.template contains_vec<Tv>(&filter_data[0],
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
            return filter.size_in_bytes();
        }
    };

}
