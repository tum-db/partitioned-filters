#pragma once

#include <cstdint>
#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include <cstring>
#include <xor/xor_parameter.hpp>
#include <array>
#include "xor_helper.hpp"

namespace filters::nxor {

    template<Variant variant, typename Hasher, typename Vector, typename Addresser>
    struct XorSet {

        using T = typename Vector::T;
        using M = typename Vector::M;

        const Addresser &addresser;
        const size_t index;
        std::array<Vector, 3> begins;
        Vector seed;
        T *set;

        XorSet(const Addresser &addresser, const Vector &seed, const size_t index) : addresser(addresser), index(index),
                                                                                     seed(seed) {
            T last_offset = 0;
            for (size_t i = 0; i < Vector::n_elements; i++) {
                const size_t n_blocks = addresser.get_size(index + i);

                for (auto &begin : begins) {
                    begin.elements[i] = last_offset;
                    last_offset += n_blocks;
                }

                if constexpr (variant == Variant::Fuse) {
                    last_offset += n_blocks * (FUSE_SEGMENTS - 1);
                }
            }

            // hold hash and counter -> multiply with 2
            size_t n_bytes = simd::valign<Vector>(last_offset * sizeof(T) * 2);
            set = simd::valloc<Vector>(n_bytes, 0);
        }

        ~XorSet() {
            free(set);
        }

        forceinline
        Vector size() const {
            if constexpr (variant == Variant::Standard) {
                return addresser.get_vsize(index);
            } else {
                return addresser.get_vsize(index) * Vector(FUSE_SEGMENTS + 2);
            }
        }

        void print() const {
            for (size_t i = 0; i < addresser.get_vsize(index).vector * (FUSE_SEGMENTS + 2); i++) {
                std::cout << i << " | " << std::hex << set[2 * i] << std::dec << " " << set[2 * i + 1] << std::endl;
            }
        }

        template<size_t set>
        forceinline
        Vector end() const {
            return begins[set] + size();
        }

        template<size_t set>
        forceinline
        Vector get_offset(const Vector &address) {
            if constexpr (Vector::avx || set > 0) {
                return (begins[set] + addresser.compute_address_horizontal(index, simd::extractAddressBits(address)));
            } else {
                return addresser.compute_address_horizontal(index, simd::extractAddressBits(address));
            }
        }

        forceinline
        Vector get_address(const Vector &offset) const {
            static_assert(variant == Variant::Fuse);
            if constexpr (Vector::avx) {
                return offset - begins[0];
            } else {
                return offset;
            }
        }

        forceinline
        Vector get_segment_size() {
            if constexpr (variant == Variant::Standard) {
                return Vector(0);
            } else {
                return addresser.get_vsize(index);
            }
        }

        template<size_t rf, size_t s>
        forceinline
        void add(const Vector &hash, const Vector &segment, const M &mask) {
            // append hash to set
            const Vector hash_offset = (segment + get_offset<s>(xor_rotate<rf>(hash))) << 1;
            (hash_offset.gather(set, mask) ^ hash).scatter(set, hash_offset, mask);

            // increment counter
            const Vector one = Vector(1);
            const Vector counter_offset = hash_offset + one;
            (counter_offset.gather(set, mask) + one).scatter(set, counter_offset, mask);
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        void fill(const Vector &offset, const Vector &histogram, const T *values) {
            Vector begin = offset;
            const Vector end = begin + histogram;
            const Vector segment_size = addresser.get_vsize(index);

            for (M mask; (mask = begin < end); begin = begin + Vector(1)) {
                const Vector value = begin.gather(values, mask);
                const Vector hash = Hasher::hash(value ^ seed);
                const Vector segment = xor_segment<variant>(hash) * segment_size;

                add<rf0, 0>(hash, segment, mask);
                add<rf1, 1>(hash, segment, mask);
                add<rf2, 2>(hash, segment, mask);
            }
        }

    };

} // filters::nxor
