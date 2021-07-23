#pragma once

#include <compiler/compiler_hints.hpp>
#include <parameter/parameter.hpp>
#include "addresser_base.hpp"

namespace filters::addresser {

    using AddressingMode = parameter::AddressingMode;

    template<typename Vector>
    struct Addresser<AddressingMode::Lemire, Vector> {

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
            const Vector sig = compute_address(index, simd::extractAddressBits(signature * Vector(0x5bd1e995)), m);
            const Vector size = index.gather(sizes, m);
            if constexpr (Vector::avx) {
                return (sig - address).add_mask(size, sig < address);
            } else {
                return sig - address + (size * Vector(sig < address));
            }
        }

        forceinline
        Vector
        compute_alternative_address_vertical(const size_t index, const Vector &address, const Vector &signature) const {
            const Vector sig = compute_address_vertical(index,
                    simd::extractAddressBits(signature * Vector(0x5bd1e995)));
            const Vector size = Vector(sizes[index]);
            if constexpr (Vector::avx) {
                return (sig - address).add_mask(size, sig < address);
            } else {
                return sig - address + (size * Vector(sig < address));
            }
        }

        forceinline
        Vector compute_alternative_address_horizontal(const size_t index, const Vector &address,
                                                      const Vector &signature) const {
            const Vector sig = compute_address_horizontal(index,
                    simd::extractAddressBits(signature * Vector(0x5bd1e995)));
            const Vector size = Vector::load(sizes + index);
            if constexpr (Vector::avx) {
                return (sig - address).add_mask(size, sig < address);
            } else {
                return sig - address + (size * Vector(sig < address));
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
