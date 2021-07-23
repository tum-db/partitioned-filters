#pragma once

#include <compiler/compiler_hints.hpp>
#include <parameter/parameter.hpp>
#include <hash/hash.hpp>
#include "addresser_base.hpp"

namespace filters::addresser {

    using AddressingMode = parameter::AddressingMode;

    template<typename Vector>
    struct Addresser<AddressingMode::Morton, Vector> {

        using T = typename Vector::T;
        using M = typename Vector::M;
        static constexpr size_t address_bits = 32;

        T *sizes;

        Addresser() : sizes(nullptr) {
        }

        Addresser(Addresser &&a) noexcept: sizes(a.sizes) {
            a.sizes = nullptr;
        };

        Addresser &operator=(Addresser &&other) noexcept {
            if (this != &other) {
                if (sizes) {
                    free(sizes);
                }
                sizes = other.sizes;
                other.sizes = nullptr;
            }
            return *this;
        }

        Addresser(const T *histogram, const size_t length) {
            sizes = simd::valloc<Vector>(simd::valign<Vector>(length * sizeof(T)), 0);
            for (size_t i = 0; i < length; i++) {
                sizes[i] = histogram[i];
            }
        }

        ~Addresser() {
            if (sizes) {
                free(sizes);
            }
        }

        forceinline
        Vector compute_address(const Vector &index, const Vector &addresses, const M m) const {
            return addresses.mulh32(index.gather(sizes, m));
        }

        forceinline
        Vector compute_address_vertical(const size_t index, const Vector &addresses) const {
            return addresses.mulh32(Vector(sizes[index]));
        }

        forceinline
        Vector compute_address_horizontal(const size_t index, const Vector &addresses) const {
            return addresses.mulh32(Vector::load(sizes + index));
        }

        forceinline
        Vector compute_alternative_address(const Vector &index, const Vector &address, const Vector &signature,
                                           const M m) const {
            const Vector one(1);
            Vector offset = hash::murmur(signature) & Vector(0x1fff) | one;
            offset = offset * (Vector(-1) + ((address & one) << one));
            const Vector size = index.gather(sizes, m);
            if constexpr (Vector::avx) {
                const M subtract = (address & one) == one;
                Vector alternate = address + offset;
                alternate = alternate.add_mask(size, subtract & (alternate >= size));
                alternate = alternate.add_mask(size * Vector(-1), (not subtract) & (alternate >= size));
                return alternate;
            } else {
                bool subtract = (address.vector & 1) == 1;
                T alternate = address.vector + offset.vector;
                return Vector(alternate + (size.vector * (-1 + (static_cast<T>(subtract) << 1))) * (alternate >= size.vector));
            }
        }

        forceinline
        Vector
        compute_alternative_address_vertical(const size_t index, const Vector &address, const Vector &signature) const {
            const Vector one(1);
            Vector offset = hash::murmur(signature) & Vector(0x1fff) | one;
            offset = offset * (Vector(-1) + ((address & one) << one));
            const Vector size = Vector(sizes[index]);
            if constexpr (Vector::avx) {
                const M subtract = (address & one) == one;
                Vector alternate = address + offset;
                alternate = alternate.add_mask(size, subtract & (alternate >= size));
                alternate = alternate.add_mask(size * Vector(-1), (not subtract) & (alternate >= size));
                return alternate;
            } else {
                bool subtract = (address.vector & 1) == 1;
                T alternate = address.vector + offset.vector;
                return Vector(alternate - (size.vector * (-1 + (static_cast<T>(subtract) << 1))) * (alternate >= size.vector));
            }
        }

        forceinline
        Vector compute_alternative_address_horizontal(const size_t index, const Vector &address,
                                                      const Vector &signature) const {
            const Vector one(1);
            Vector offset = hash::murmur(signature) & Vector(0x1fff) | one;
            offset = offset * (Vector(-1) + ((address & one) << one));
            const Vector size = Vector::load(sizes + index);
            if constexpr (Vector::avx) {
                const M subtract = (address & one) == one;
                Vector alternate = address + offset;
                alternate = alternate.add_mask(size, subtract & (alternate >= size));
                alternate = alternate.add_mask(size * Vector(-1), (not subtract) & (alternate >= size));
                return alternate;
            } else {
                bool subtract = (address.vector & 1) == 1;
                T alternate = address.vector + offset.vector;
                return Vector(alternate - (size.vector * (-1 + (static_cast<T>(subtract) << 1))) * (alternate >= size.vector));
            }
        }

        forceinline
        size_t get_size(size_t index) const {
            return sizes[index];
        }

        forceinline
        Vector get_vsize(size_t index) const {
            return Vector::load(sizes + index);
        }

        template<size_t n>
        forceinline
        Vector fingerprint(const Vector &hash) const {
            return ((hash >> (static_cast<size_t>(Vector::registerSize) - n)) ^ hash) & Vector((1ull << n) - 1);
        }

    };

} // filters::addresser
