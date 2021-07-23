#pragma once

#include <bloom/bloom_parameter.hpp>
#include <hash/hash.hpp>
#include <math/math.hpp>
#include "bloom_mask_base.hpp"

namespace filters::bloom {

    template<Groups groups, BlockSize blockSize, size_t k, RegisterSize registerSize, SIMD _simd, size_t remaining_k, size_t _remaining_hash_bits, size_t _n_rehash,
            typename Hasher>
    struct BloomMask<Variant::Blocked, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd,
            remaining_k, _remaining_hash_bits, _n_rehash, Hasher> {

        using Vector = simd::Vector<registerSize, _simd>;
        using Mask = typename Vector::M;

        static constexpr size_t n_lanes = Vector::n_elements;

        static constexpr size_t word_size = static_cast<size_t>(registerSize);
        static constexpr size_t word_bits = math::const_log2(word_size);
        static constexpr size_t n_sectors = static_cast<size_t >(blockSize) / word_size;
        static constexpr size_t sector_bits = math::const_log2(n_sectors);
        static constexpr size_t hash_bits_per_iteration = sector_bits + word_bits;
        static constexpr size_t remaining_hash_bits = (_remaining_hash_bits < hash_bits_per_iteration)
                                                      ? Hasher::hash_bits
                                                      : _remaining_hash_bits;
        static constexpr size_t hash_offset = Hasher::hash_bits - remaining_hash_bits;
        static constexpr size_t n_rehash = _n_rehash + (_remaining_hash_bits < hash_bits_per_iteration);

        using Recursion = BloomMask<Variant::Blocked, Vectorization::Horizontal, blockSize, groups, k, registerSize,
                _simd, remaining_k - 1, remaining_hash_bits - hash_bits_per_iteration, n_rehash, Hasher>;

        forceinline
        static void
        generate_mask(const Vector &values, Vector &hash, void *base_address, const Vector &offset, const Mask mask) {
            if constexpr (_remaining_hash_bits < hash_bits_per_iteration) {
                hash = Hasher::template rehash<_n_rehash>(hash, values);
            }

            const Vector sector_address = offset + simd::extractBits(hash, hash_offset, sector_bits);

            Vector vector = sector_address.gather(base_address, mask);
            vector = vector | (Vector(1) << simd::extractBits(hash, hash_offset + sector_bits, word_bits));
            vector.scatter(base_address, sector_address, mask);

            if constexpr (remaining_k > 1) {
                Recursion::generate_mask(values, hash, base_address, offset, mask);
            }
        }

        forceinline
        static Mask check_mask(const Vector &values, Vector &hash, const void *base_address, const Vector &offset,
                               const Mask mask) {
            if constexpr (_remaining_hash_bits < hash_bits_per_iteration) {
                hash = Hasher::template rehash<_n_rehash>(hash, values);
            }

            const Vector sector_address = offset + simd::extractBits(hash, hash_offset, sector_bits);

            const Vector vector = sector_address.gather(base_address, mask);
            const Mask found =
                    ((vector >> simd::extractBits(hash, hash_offset + sector_bits, word_bits)) & Vector(1)) ==
                    Vector(1);

            if constexpr (remaining_k > 1) {
                if constexpr ((remaining_k - 1) % 8 == 0) {
                    if (found == 0) {
                        return 0;
                    }
                }

                return found & Recursion::check_mask(values, hash, base_address, offset, mask);
            } else {
                return mask & found;
            }
        }
    };

} // filters::bloom
