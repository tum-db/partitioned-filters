#pragma once

#include <compiler/compiler_hints.hpp>
#include <cstddef>

namespace filters::nxor {

    template<size_t rotating_factor, typename Vector>
    forceinline
    static Vector xor_rotate(const Vector &vector) {
        return vector.template rol<rotating_factor>();
    }

    template<typename Vector>
    forceinline
    static Vector xor_fingerprint(const Vector &vector) {
        return vector ^ (vector >> (static_cast<size_t>(Vector::registerSize) / 2));
    }

    template<Variant variant, typename Vector>
    forceinline
    static Vector xor_segment(const Vector &vector) {
        if constexpr (variant == Variant::Standard) {
            return Vector(0);
        } else if constexpr (Vector::registerSize == parameter::RegisterSize::_64bit) {
            // taken from http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
            return (vector * Vector(0xbf58476d1ce4e5b9))
                    >> (static_cast<size_t>(Vector::registerSize) - math::const_log2(FUSE_SEGMENTS));
        } else {
            // taken from https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp
            return (vector * Vector(0x5bd1e995))
                    >> (static_cast<size_t>(Vector::registerSize) - math::const_log2(FUSE_SEGMENTS));
        }
    }

    template<Variant variant>
    forceinline
    static size_t xor_number_of_segment() {
        if constexpr (variant == Variant::Standard) {
            return 0;
        } else {
            return FUSE_SEGMENTS;
        }
    }
}