#pragma once

#include <cstddef>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include "hash.hpp"
#include "hasher_base.hpp"
#include <cityhash/city.h>

namespace filters::hash {

    using HashingMode = parameter::HashingMode;
    using RegisterSize = parameter::RegisterSize;

    template<typename Vector, size_t config>
    struct Hasher<HashingMode::Cityhash, Vector, config> {

        static constexpr size_t hash_bits = static_cast<size_t>(Vector::registerSize);

        forceinline
        static Vector hash(const Vector &val) {
            Vector res(0);
            for (size_t i = 0; i < Vector::n_elements; i++) {
                if constexpr (Vector::registerSize == RegisterSize::_32bit) {
                    res.elements[i] = CityHash32(reinterpret_cast<const char *>(&val.elements[i]),
                            sizeof(typename Vector::T));
                } else {
                    res.elements[i] = CityHash64(reinterpret_cast<const char *>(&val.elements[i]),
                            sizeof(typename Vector::T));
                }
            }
            return res;
        }

        template<size_t>
        forceinline
        static Vector rehash(const Vector &hash, const Vector &val) {
            if constexpr (Vector::registerSize == RegisterSize::_32bit) {
                return hash(hash ^ val);
            } else {
                Vector res(0);
                for (size_t i = 0; i < Vector::n_elements; i++) {
                    res.elements[i] = CityHash64WithSeed(reinterpret_cast<const char *>(&val.elements[i]),
                            sizeof(typename Vector::T), hash.elements[i]);
                }
                return res;
            }
        }
    };

} // filters::hash