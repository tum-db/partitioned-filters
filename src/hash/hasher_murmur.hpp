#pragma once

#include <cstddef>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include "hash.hpp"
#include "hasher_base.hpp"

namespace filters::hash {

    using HashingMode = parameter::HashingMode;

    template<typename Vector, size_t config>
    struct Hasher<HashingMode::Murmur, Vector, config> {

        static constexpr size_t hash_bits = static_cast<size_t>(Vector::registerSize);

        forceinline
        static Vector hash(const Vector &val) {
            return murmur(val);
        }

        template<size_t>
        forceinline
        static Vector rehash(const Vector &hash, const Vector &val) {
            return murmur(hash ^ val);
        }
    };

} // filters::hash