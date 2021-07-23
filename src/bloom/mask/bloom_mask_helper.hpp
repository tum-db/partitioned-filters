#pragma once

#include <math/math.hpp>
#include <parameter/parameter.hpp>
#include <simd/vector.hpp>
#include <simd/helper.hpp>

namespace filters::bloom {

    using RegisterSize = parameter::RegisterSize;
    using SIMD = parameter::SIMD;

    template<RegisterSize registerSize, SIMD _simd,
            typename Hasher, size_t remaining_k, size_t _remaining_hash_bits, size_t _n_rehash>
    struct BloomMaskHelper {

        using Vector = simd::Vector<registerSize, _simd>;

        static constexpr size_t word_size = static_cast<size_t>(registerSize);
        static constexpr size_t word_bits = math::const_log2(word_size);
        static constexpr size_t start_remaining_hash_bits = (_remaining_hash_bits < word_bits)
                                                            ? Hasher::hash_bits
                                                            : _remaining_hash_bits;
        static constexpr size_t hash_offset = Hasher::hash_bits - start_remaining_hash_bits;
        static constexpr size_t start_n_rehash = _n_rehash + (_remaining_hash_bits < word_bits);

        using Recursion = BloomMaskHelper<registerSize, _simd, Hasher, remaining_k - 1,
                start_remaining_hash_bits - word_bits, start_n_rehash>;

        static constexpr size_t remaining_hash_bits = Recursion::remaining_hash_bits;
        static constexpr size_t n_rehash = Recursion::n_rehash;

        forceinline static void fill_mask(const Vector &values, Vector &hash, Vector &vector) {
            if constexpr (_remaining_hash_bits < word_bits) {
                hash = Hasher::template rehash<_n_rehash>(hash, values);
            }

            vector = vector | (Vector(1) << simd::extractBits(hash, hash_offset, word_bits));

            Recursion::fill_mask(values, hash, vector);
        }
    };

    template<parameter::RegisterSize registerSize, SIMD _simd,
            typename Hasher, size_t _remaining_hash_bits, size_t _n_rehash>
    struct BloomMaskHelper<registerSize, _simd, Hasher, 0, _remaining_hash_bits, _n_rehash> {

        using Vector = simd::Vector<registerSize, _simd>;

        static constexpr size_t remaining_hash_bits = _remaining_hash_bits;
        static constexpr size_t n_rehash = _n_rehash;

        forceinline static void fill_mask(const Vector &, Vector &, Vector &) {
            // end of recursion
        }
    };

} // filters::bloom
