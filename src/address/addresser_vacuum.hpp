#pragma once

#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include "addresser_base.hpp"

namespace filters::addresser {

    using AddressingMode = parameter::AddressingMode;

    template<typename Vector>
    struct Addresser<AddressingMode::Vacuum, Vector> {

        using T = typename Vector::T;
        using M = typename Vector::M;
        static constexpr size_t address_bits = 32;

        T *sizes;
        T *masks;

        Addresser() : sizes(nullptr), masks(nullptr) {
        }

        Addresser(Addresser &&a) noexcept: sizes(a.sizes), masks(a.masks) {
            a.sizes = nullptr;
            a.masks = nullptr;
        };

        Addresser &operator=(Addresser &&other) noexcept {
            if (this != &other) {
                if (sizes) {
                    free(sizes);
                }
                if (masks) {
                    free(masks);
                }
                sizes = other.sizes;
                masks = other.masks;
                other.sizes = nullptr;
                other.masks = nullptr;
            }
            return *this;
        }

        Addresser(const T *histogram, const size_t length) {
            sizes = simd::valloc<Vector>(simd::valign<Vector>(length * sizeof(T)), 0);
            masks = simd::valloc<Vector>(simd::valign<Vector>(length * sizeof(T)), 0);
            for (size_t i = 0; i < length; i++) {
                // fix number of chunks to max 16
                T chunk_size = math::next_power_of_two(histogram[i]) / 16;

                // make sure each chunk contains at least 256 entries
                if (chunk_size >= 256) {
                    sizes[i] = ((histogram[i] + chunk_size - 1) / chunk_size) * chunk_size;
                    masks[i] = chunk_size - 1;
                } else {
                    sizes[i] = math::next_power_of_two(histogram[i]);
                    masks[i] = sizes[i] - 1;
                }
            }
        }

        ~Addresser() {
            if (sizes) {
                free(sizes);
            }
            if (masks) {
                free(masks);
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
            return address ^ (simd::extractAddressBits(signature * Vector(0x5bd1e995)) & index.gather(masks, m));
        }

        forceinline
        Vector
        compute_alternative_address_vertical(const size_t index, const Vector &address, const Vector &signature) const {
            return address ^ (simd::extractAddressBits(signature * Vector(0x5bd1e995)) & Vector(masks[index]));
        }

        forceinline
        Vector compute_alternative_address_horizontal(const size_t index, const Vector &address,
                                                      const Vector &signature) const {
            return address ^ (simd::extractAddressBits(signature * Vector(0x5bd1e995)) & Vector::load(masks + index));
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