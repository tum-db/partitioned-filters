#pragma once

#include <cstddef>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include "hash.hpp"
#include "hasher_base.hpp"

namespace filters::hash {

    using HashingMode = parameter::HashingMode;

    template<typename Vector, size_t config>
    struct Hasher<HashingMode::Mul, Vector, config> {

        static constexpr size_t hash_bits = static_cast<size_t>(Vector::registerSize);

        static constexpr size_t n_factors = 8;

        static constexpr uint64_t factors64[n_factors] = {0xc3a5c85c97cb3127, // CityHash
                                                          0xb492b66fbe98f273, // CityHash
                                                          0x9ae16a3b2f90404f, // CityHash
                                                          0xc949d7c7509e6557, // CityHash
                                                          0xff51afd7ed558ccd, // Murmur3
                                                          0xc4ceb9fe1a85ec53, // Murmur3
                                                          0x87c37b91114253d5, // Murmur3
                                                          0x4cf5ad432745937f, // Murmur3
        };

        static constexpr uint32_t factors32[n_factors] = {0x85ebca6b, // Murmur3
                                                          0xc2b2ae35, // Murmur3
                                                          0xcc9e2d51, // Murmur3
                                                          0x1b873593, // Murmur3
                                                          0x239b961b, // Murmur3
                                                          0xab0e9789, // Murmur3
                                                          0x38b34ae5, // Murmur3
                                                          0xa1e38b93, // Murmur3
        };

        forceinline
        static Vector hash(const Vector &val) {
            if constexpr (Vector::registerSize == parameter::RegisterSize::_32bit) {
                return val * Vector(factors32[0]);
            } else {
                return val * Vector(factors64[0]);
            }
        }

        template<size_t i>
        forceinline
        static Vector rehash(const Vector &hash, const Vector &val) {
            static_assert(i + 1 < n_factors, "not enough multiplication factors!");
            if constexpr (Vector::registerSize == parameter::RegisterSize::_32bit) {
                return val * Vector(factors32[i + 1]);
            } else {
                return val * Vector(factors64[i + 1]);
            }
        }
    };

} // filters::hash