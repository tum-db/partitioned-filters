#pragma once

#include <bloom/bloom_parameter.hpp>
#include <hash/hash.hpp>
#include "bloom_mask_base.hpp"
#include "bloom_mask_helper.hpp"

namespace filters::bloom {

    template<Groups groups, BlockSize blockSize, size_t k, RegisterSize registerSize, SIMD _simd, size_t remaining_k, size_t remaining_hash_bits, size_t n_rehash,
            typename Hasher>
    struct BloomMask<Variant::Sectorized, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd,
            remaining_k, remaining_hash_bits, n_rehash, Hasher> {

        using Vector = simd::Vector<registerSize, _simd>;
        using Mask = typename Vector::M;
        using T = typename Vector::T;

        static constexpr size_t n_lanes = Vector::n_elements;

        static constexpr size_t word_size = static_cast<size_t>(registerSize);
        static constexpr size_t word_bits = math::const_log2(word_size);
        static constexpr size_t n_sectors = static_cast<size_t >(blockSize) / word_size;
        static constexpr size_t sector_bits = math::const_log2(n_sectors);
        static constexpr size_t k_per_iteration = k / n_sectors;

        static_assert(k % n_sectors == 0, "k must be a multiple of the number of sectors!");

        using Helper = BloomMaskHelper<registerSize, _simd, Hasher, k_per_iteration, remaining_hash_bits, n_rehash>;
        using Recursion = BloomMask<Variant::Sectorized, Vectorization::Horizontal, blockSize, groups, k, registerSize,
                _simd, remaining_k - k_per_iteration, Helper::remaining_hash_bits, Helper::n_rehash, Hasher>;

        forceinline
        static void
        generate_mask(const Vector &values, Vector &hash, void *base_address, const Vector &offset, const Mask mask) {
            Vector vector = offset.gather(base_address, mask);
            Helper::fill_mask(values, hash, vector);
            vector.scatter(base_address, offset, mask);

            if constexpr (remaining_k > k_per_iteration) {
                Recursion::generate_mask(values, hash, reinterpret_cast<T *>(base_address) + 1, offset, mask);
            }
        }

        forceinline
        static Mask check_mask(const Vector &values, Vector &hash, const void *base_address, const Vector &offset,
                               const Mask mask) {
            Vector vmask = Vector(0);
            Helper::fill_mask(values, hash, vmask);

            const Vector vector = offset.gather(base_address, mask);
            const Mask found = ((vector & vmask) == vmask);

            if constexpr (remaining_k > k_per_iteration) {
                if (found == 0) {
                    return 0;
                }

                return found &
                       Recursion::check_mask(values, hash, reinterpret_cast<const T *>(base_address) + 1, offset, mask);
            } else {
                return mask & found;
            }
        }

    };

} // filters::bloom
