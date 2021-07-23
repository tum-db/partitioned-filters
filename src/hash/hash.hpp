#pragma once

#include <glob.h>
#include <cstdint>
#include <cassert>
#include <parameter/parameter.hpp>

namespace filters::hash {

    template<typename Vector>
    static Vector murmur(const Vector &v) {
        if constexpr (Vector::registerSize == parameter::RegisterSize::_32bit) {
            const Vector mul1(0x85ebca6b), mul2(0xc2b2ae35);

            Vector h = v;
            h = h ^ (h >> 16);
            h = h * mul1;
            h = h ^ (h >> 13);
            h = h * mul2;
            h = h ^ (h >> 16);

            return h;
        } else {
            const Vector mul1(0xff51afd7ed558ccd), mul2(0xc4ceb9fe1a85ec53);

            Vector h = v;
            h = h ^ (h >> 33);
            h = h * mul1;
            h = h ^ (h >> 33);
            h = h * mul2;
            h = h ^ (h >> 33);

            return h;
        }
    }

    template<typename Vector>
    static Vector fasthash(const Vector &v) {
        if constexpr (Vector::registerSize == parameter::RegisterSize::_32bit) {
            const Vector mul1(0x21275c37), mul2(0x88031965);

            Vector h = v;
            h = h ^ (h >> 11);
            h = h * mul1;
            h = h ^ (h >> 23);

            return h;
        } else {
            const Vector mul1(0x2127599bf4325c37), mul2(0x880355f21e6d1965);

            Vector h = v;
            h = h ^ (h >> 23);
            h = h * mul1;
            h = h ^ (h >> 47);

            return h;
        }
    }

} // filters::hash
