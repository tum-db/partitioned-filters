#pragma once

#include <cstddef>
#include <math/math.hpp>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include <simd/vector.hpp>

namespace filters::hash {

    // randomly generated numbers
    static constexpr uint64_t seed64[16] = {0xb50ee07e66a4cebc, 0x5aded764e5520ef4, 0xd44415b984b89b63,
                                            0xd83bed13657b40b3, 0x7ee34367bd4b7901, 0x41bb0212d77eeb8b,
                                            0xc3049db8cb628eb7, 0x7940a3ce9ab66c15, 0x37d599e2efa012cb,
                                            0xfa983f3693b58352, 0x9f9216f3b0218c4f, 0x2c50b7a21d3fed06,
                                            0xe0eba29b8bdd2b50, 0x64732b59505022f0, 0x3a9f118b227040e7,
                                            0x34aa4d71c272549b};

    static constexpr uint32_t seed32[16] = {0xcdeb703e, 0x133ec928, 0x317ce7dc, 0xea8edcad, 0x52c7be8e, 0x62790d19,
                                            0x7d6190ca, 0x0950a40f, 0xec0702aa, 0x9ad9b931, 0xb94f9e6a, 0xb7d48e38,
                                            0xaea82df5, 0xd2f08779, 0xcaf0d659, 0x2c55ae68};

    forceinline
    static size_t n_seeds() noexcept {
        return 16;
    }

    template<typename Vector>
    forceinline
    static Vector single_seed(const size_t index) noexcept {
        if constexpr (Vector::registerSize == parameter::RegisterSize::_32bit) {
            return Vector(seed32[index]);
        } else {
            return Vector(seed64[index]);
        }
    }

    template<typename Vector, size_t n_lanes>
    forceinline
    static Vector seed() noexcept {
        Vector v(0);
        for (size_t i = 0; i < n_lanes; i++) {
            for (size_t j = 0; j < Vector::n_elements / n_lanes; j++) {
                if constexpr (Vector::registerSize == parameter::RegisterSize::_32bit) {
                    v.elements[i * (Vector::n_elements / n_lanes) + j] = seed32[j];
                } else {
                    v.elements[i * (Vector::n_elements / n_lanes) + j] = seed64[j];
                }
            }
        }
        return v;
    }

} // filters::hash