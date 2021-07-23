#pragma once

#include <bloom/bloom_parameter.hpp>
#include <hash/hash.hpp>
#include "bloom_mask_base.hpp"
#include "bloom_mask_helper.hpp"

namespace filters::bloom {

    template<Groups groups, BlockSize blockSize, size_t k, RegisterSize registerSize, SIMD _simd, size_t remaining_k, size_t _remaining_hash_bits, size_t _n_rehash,
            typename Hasher>
    struct BloomMask<Variant::Grouped, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd,
            remaining_k, _remaining_hash_bits, _n_rehash, Hasher> {

        using Vector = simd::Vector<registerSize, _simd>;
        using Mask = typename Vector::M;

        static constexpr size_t n_lanes = Vector::n_elements;

        static constexpr size_t word_size = static_cast<size_t>(registerSize);
        static constexpr size_t word_bits = math::const_log2(word_size);
        static constexpr size_t n_groups = static_cast<size_t >(groups);
        static constexpr size_t n_sectors_per_group = static_cast<size_t >(blockSize) / word_size / n_groups;
        static constexpr size_t n_sector_bits_per_group = math::const_log2(n_sectors_per_group);
        static constexpr size_t k_per_iteration = k / n_groups;
        static constexpr size_t remaining_hash_bits = (_remaining_hash_bits < n_sector_bits_per_group)
                                                      ? Hasher::hash_bits
                                                      : _remaining_hash_bits;
        static constexpr size_t hash_offset = Hasher::hash_bits - remaining_hash_bits;
        static constexpr size_t n_rehash = _n_rehash + (_remaining_hash_bits < n_sector_bits_per_group);

        using Helper = BloomMaskHelper<registerSize, _simd, Hasher, k_per_iteration,
                remaining_hash_bits - n_sector_bits_per_group, n_rehash>;
        using Recursion = BloomMask<Variant::Grouped, Vectorization::Horizontal, blockSize, groups, k, registerSize,
                _simd, remaining_k - k_per_iteration, Helper::remaining_hash_bits, Helper::n_rehash, Hasher>;

        forceinline
        static void
        generate_mask(const Vector &values, Vector &hash, void *base_address, const Vector &offset, const Mask mask) {
            if constexpr (_remaining_hash_bits < n_sector_bits_per_group) {
                hash = Hasher::template rehash<_n_rehash>(hash, values);
            }

            const size_t group_address = n_sectors_per_group * (n_groups - remaining_k / k_per_iteration);
            const Vector sector_address =
                    offset + Vector(group_address) + simd::extractBits(hash, hash_offset, n_sector_bits_per_group);

            Vector vector = sector_address.gather(base_address, mask);
            Helper::fill_mask(values, hash, vector);
            vector.scatter(base_address, sector_address, mask);

            if constexpr (remaining_k > k_per_iteration) {
                Recursion::generate_mask(values, hash, base_address, offset, mask);
            }
        }

        forceinline
        static Mask check_mask(const Vector &values, Vector &hash, const void *base_address, const Vector &offset,
                               const Mask mask) {
            if constexpr (_remaining_hash_bits < n_sector_bits_per_group) {
                hash = Hasher::template rehash<_n_rehash>(hash, values);
            }

            const size_t group_address = n_sectors_per_group * (n_groups - remaining_k / k_per_iteration);
            const Vector sector_address =
                    offset + Vector(group_address) + simd::extractBits(hash, hash_offset, n_sector_bits_per_group);

            Vector vmask = Vector(0);
            Helper::fill_mask(values, hash, vmask);

            const Vector vector = sector_address.gather(base_address, mask);
            const Mask found = ((vector & vmask) == vmask);

            if constexpr (remaining_k > k_per_iteration) {
                if (found == 0) {
                    return 0;
                }

                return found & Recursion::check_mask(values, hash, base_address, offset, mask);
            } else {
                return mask & found;
            }
        }
    };

} // filters::bloom
