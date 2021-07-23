#pragma once

#include <immintrin.h>
#include <simd/vector.hpp>

namespace filters::bloom {

    using RegisterSize = parameter::RegisterSize;
    using SIMD = parameter::SIMD;

    template<Variant, Vectorization, BlockSize, Groups, size_t, RegisterSize registerSize, SIMD _simd, size_t, size_t, size_t,
            typename>
    struct BloomMask {

        using Vector = simd::Vector<registerSize, _simd>;
        using Mask = typename Vector::M;

        forceinline
        static void generate_mask(const Vector &, Vector &, void *, const Vector &, const Mask) {
            throw std::logic_error{"not implemented!"};
        }

        forceinline
        static Mask check_mask(const Vector &, Vector &, const void *, const Vector &, const Mask) {
            throw std::logic_error{"not implemented!"};
        }
    };

} // filters::bloom
