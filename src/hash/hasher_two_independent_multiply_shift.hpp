#pragma once

#include <cstddef>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include "hash.hpp"
#include "hasher_base.hpp"

namespace filters::hash {

    using HashingMode = parameter::HashingMode;
    using RegisterSize = parameter::RegisterSize;

    template<typename Vector, size_t config>
    struct Hasher<HashingMode::TwoIndependentMultiplyShift, Vector, config> {

        static_assert(not Vector::avx, "AVX not supported!");

        static constexpr size_t hash_bits = static_cast<size_t>(Vector::registerSize);

        forceinline
        static Vector hash(const Vector &val) {
            if constexpr (Vector::registerSize == RegisterSize::_32bit) {
                return Vector(
                        (0xc4ceb9fe1a85ec53ull + 0xff51afd7ed558ccdull * static_cast<uint64_t>(val.vector)) >> 32);
            } else {
                constexpr __int128 multiply =
                        (static_cast<__int128>(0xc3049db8cb628eb7ull) << 64) + 0x3a9f118b227040e7ull;
                constexpr __int128 add = (static_cast<__int128>(0xc4ceb9fe1a85ec53ull) << 64) + 0xff51afd7ed558ccdull;
                return Vector((add + multiply * static_cast<__int128>(val.vector)) >> 64);

            }
        }

        template<size_t>
        forceinline
        static Vector rehash(const Vector &hash, const Vector &val) {
            return Hasher::hash(hash ^ val);
        }
    };

} // filters::hash