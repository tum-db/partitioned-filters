#pragma once

#include <cstdint>
#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include "xor_set.hpp"

namespace filters::nxor {

    template<Variant variant, typename Hasher, typename Vector, typename Addresser>
    struct XorQueue {
    };

    template<typename Hasher, typename Vector, typename Addresser>
    struct XorQueue<Variant::Standard, Hasher, Vector, Addresser> {

        using Set = XorSet<Variant::Standard, Hasher, Vector, Addresser>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        std::array<Vector, 3> begins;
        std::array<Vector, 3> ends;
        T *queue;

        XorQueue(const Addresser &addresser, const size_t index) {
            T last_offset = 0;
            for (size_t i = 0; i < Vector::n_elements; i++) {
                const size_t n_blocks = addresser.get_size(index + i);

                for (size_t j = 0; j < begins.size(); j++) {
                    begins[j].elements[i] = last_offset;
                    ends[j].elements[i] = last_offset;
                    last_offset += n_blocks;
                }
            }

            size_t n_bytes = simd::valign<Vector>(last_offset * sizeof(T));
            queue = simd::valloc<Vector>(n_bytes);
        }

        ~XorQueue() {
            free(queue);
        }

        template<size_t set>
        forceinline
        void clear() {
            ends[set] = begins[set];
        }

        template<size_t set>
        forceinline
        Vector size() const {
            return ends[set] - begins[set];
        }

        template<size_t set>
        forceinline
        void scan_for_single(const Set &xor_set) {
            const Vector one = Vector(1);
            const Vector two = Vector(2);

            // begin of current set (needs to be incremented by factor 2)
            Vector set_begin = xor_set.begins[set] << 1;
            const Vector set_length = xor_set.size();
            const Vector set_end = set_begin + (set_length << 1);

            // end of current queue
            Vector &queue_ends = ends[set];
            for (M mask; (mask = set_begin < set_end); set_begin = set_begin + two) {
                const Vector entry_counter = (set_begin + one).gather(xor_set.set, mask);
                mask &= (entry_counter == one);

                // for scalar we need to check the mask manually
                if constexpr (not Vector::avx) {
                    if (not mask) {
                        continue;
                    }
                }

                const Vector entry_hash = set_begin.gather(xor_set.set, mask);

                // we could write offsets into the queue
                // but the hash is more effective
                // when building the stack we reduce the number of loads on the critical path to one
                // when using the offset we first have to load the hash and when we perform
                // another load based on the address computed from the hash
                entry_hash.scatter(queue, queue_ends, mask);
                // increment size of xor queue
                queue_ends = queue_ends.add_mask(one, mask);
            }
        }

        forceinline
        void fill(const Set &xor_set) {
            scan_for_single<0>(xor_set);
            scan_for_single<1>(xor_set);
            scan_for_single<2>(xor_set);
        }
    };

    template<typename Hasher, typename Vector, typename Addresser>
    struct XorQueue<Variant::Fuse, Hasher, Vector, Addresser> {

        using Set = XorSet<Variant::Fuse, Hasher, Vector, Addresser>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        Vector begins;
        Vector ends;
        T *queue;

        XorQueue(const Addresser &addresser, const size_t index) {
            T last_offset = 0;
            for (size_t i = 0; i < Vector::n_elements; i++) {
                const size_t n_blocks = addresser.get_size(index + i);
                begins.elements[i] = last_offset;
                ends.elements[i] = last_offset;
                last_offset += n_blocks * (FUSE_SEGMENTS + 2);
            }

            size_t n_bytes = simd::valign<Vector>(last_offset * sizeof(T));
            queue = simd::valloc<Vector>(n_bytes);
        }

        ~XorQueue() {
            free(queue);
        }

        forceinline
        void clear() {
            ends = begins;
        }

        forceinline
        Vector size() const {
            return ends - begins;
        }

        forceinline
        void fill(const Set &xor_set) {
            const Vector one = Vector(1);
            const Vector two = Vector(2);

            // begin of current set (needs to be incremented by factor 2)
            Vector set_begin = xor_set.begins[0] << 1;
            const Vector set_length = xor_set.size();
            const Vector set_end = set_begin + (set_length << 1);

            for (M mask; (mask = set_begin < set_end); set_begin = set_begin + two) {
                const Vector entry_counter = (set_begin + one).gather(xor_set.set, mask);
                mask &= (entry_counter == one);

                // for scalar we need to check the mask manually
                if constexpr (not Vector::avx) {
                    if (not mask) {
                        continue;
                    }
                }

                xor_set.get_address(set_begin >> 1).scatter(queue, ends, mask);
                // increment size of xor queue
                ends = ends.add_mask(one, mask);
            }
        }
    };

} // filters::nxor
