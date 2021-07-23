#pragma once

#include <libdivide.h>
#include <compiler/compiler_hints.hpp>
#include <parameter/parameter.hpp>
#include "addresser_base.hpp"

namespace filters::addresser {

    using AddressingMode = parameter::AddressingMode;

    template<typename Vector>
    struct Addresser<AddressingMode::Magic, Vector> {

        using T = typename Vector::T;
        using M = typename Vector::M;
        static constexpr size_t address_bits = 32;

        T *magics;
        T *shift_amounts;
        T *divisors;

        Addresser() : magics(nullptr), shift_amounts(nullptr), divisors(nullptr) {
        }

        Addresser(Addresser &&a) noexcept: magics(a.magics), shift_amounts(a.shift_amounts), divisors(a.divisors) {
            a.magics = nullptr;
            a.shift_amounts = nullptr;
            a.divisors = nullptr;
        };

        Addresser &operator=(Addresser &&other) noexcept {
            if (this != &other) {
                if (magics) {
                    free(magics);
                }
                if (shift_amounts) {
                    free(shift_amounts);
                }
                if (divisors) {
                    free(divisors);
                }

                magics = other.magics;
                shift_amounts = other.shift_amounts;
                divisors = other.divisors;

                other.magics = nullptr;
                other.shift_amounts = nullptr;
                other.divisors = nullptr;
            }
            return *this;
        }

        Addresser(const T *histogram, const size_t length) {
            magics = simd::valloc<Vector>(simd::valign<Vector>(length * sizeof(T)), 0);
            shift_amounts = simd::valloc<Vector>(simd::valign<Vector>(length * sizeof(T)), 0);
            divisors = simd::valloc<Vector>(simd::valign<Vector>(length * sizeof(T)), 0);

            for (size_t i = 0; i < length; i++) {
                auto div = libdivide::libdivide_u32_cheap_gen(histogram[i]);
                auto d = libdivide::libdivide_u32_recover(&div);
                magics[i] = static_cast<T>(div.magic);
                shift_amounts[i] = static_cast<T>(div.more);
                divisors[i] = static_cast<T>(d);
            }
        }

        ~Addresser() {
            if (magics) {
                free(magics);
            }
            if (shift_amounts) {
                free(shift_amounts);
            }
            if (divisors) {
                free(divisors);
            }
        }

        forceinline
        Vector compute_address(const Vector &index, const Vector &addresses, const M m) const {
            const Vector magic = index.gather(magics, m);
            const Vector shift_amount = index.gather(shift_amounts, m);
            const Vector divisor = index.gather(divisors, m);
            const Vector quotient = addresses.mulh32(magic);
            return addresses - (quotient >> shift_amount) * divisor;
        }

        forceinline
        Vector compute_address_vertical(const size_t &index, const Vector &addresses) const {
            const Vector magic = Vector(magics[index]);
            const Vector shift_amount = Vector(shift_amounts[index]);
            const Vector divisor = Vector(divisors[index]);
            const Vector quotient = addresses.mulh32(magic);
            return addresses - (quotient >> shift_amount) * divisor;
        }

        forceinline
        Vector compute_address_horizontal(const size_t &index, const Vector &addresses) const {
            const Vector magic = Vector::load(magics + index);
            const Vector shift_amount = Vector::load(shift_amounts + index);
            const Vector divisor = Vector::load(divisors + index);
            const Vector quotient = addresses.mulh32(magic);
            return addresses - (quotient >> shift_amount) * divisor;
        }

        forceinline
        Vector compute_alternative_address(const Vector &index, const Vector &address, const Vector &signature,
                                           const M m) const {
            const Vector sig = compute_address(index, simd::extractAddressBits(signature * Vector(0x5bd1e995)), m);
            const Vector size = index.gather(divisors, m);
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
            const Vector size = Vector(divisors[index]);
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
            const Vector size = Vector::load(divisors + index);
            if constexpr (Vector::avx) {
                return (sig - address).add_mask(size, sig < address);
            } else {
                return sig - address + (size * Vector(sig < address));
            }
        }

        forceinline
        size_t get_size(size_t index) const {
            return divisors[index];
        }

        forceinline
        Vector get_vsize(size_t index) const {
            return Vector::load(divisors + index);
        }

        template<size_t n>
        forceinline
        Vector fingerprint(const Vector &hash) const {
            return ((hash >> (static_cast<size_t>(Vector::registerSize) - n)) ^ hash) & Vector((1ull << n) - 1);
        }

    };

} // filters::addresser