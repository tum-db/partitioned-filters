#pragma once

#include <cstdint>
#include <cstddef>
#include <bloom/bloom_parameter.hpp>

namespace filters::nxor {

    template<Variant variant, typename Hasher, typename Vector, typename Addresser>
    struct XorStack {
    };

    template<typename Hasher, typename Vector, typename Addresser>
    struct XorStack<Variant::Standard, Hasher, Vector, Addresser> {

        using Set = XorSet<Variant::Standard, Hasher, Vector, Addresser>;
        using Queue = XorQueue<Variant::Standard, Hasher, Vector, Addresser>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        // list of runs, store offset into the stack
        std::vector<Vector> runs;
        T *stack;

        XorStack(const Vector &histogram) {
            Vector first_run{0};
            T last_offset = 0;
            for (size_t i = 0; i < Vector::n_elements; i++) {
                const size_t n_entries = histogram.elements[i];
                first_run.elements[i] = last_offset;
                last_offset += n_entries;
            }
            runs.push_back(first_run);

            size_t n_bytes = simd::valign<Vector>(last_offset * sizeof(T));
            stack = simd::valloc<Vector>(n_bytes);
        }

        ~XorStack() {
            free(stack);
        }

        forceinline
        void start_run() {
            runs.push_back(runs.back());
        }

        forceinline
        Vector size() const {
            return runs.back() - runs.front();
        }

        template<size_t set0, size_t rf0, size_t set1, size_t rf1, size_t set2, size_t rf2>
        forceinline
        void process_queue(Set &set, Queue &queue) {
            start_run();

            const Vector one = Vector(1);

            // begin, length and end of current queue (needs to be incremented by factor 2)
            Vector queue0_begin = queue.begins[set0];
            const Vector queue0_end = queue.ends[set0];

            // end of other two queues, needed for appending
            Vector &queue1_end = queue.ends[set1];
            Vector &queue2_end = queue.ends[set2];

            // offset into stack
            Vector &run = runs.back();
            for (M mask; (mask = queue0_begin < queue0_end); queue0_begin = queue0_begin + one) {
                const Vector entry0_hash = queue0_begin.gather(queue.queue, mask);
                const Vector entry0_offset = set.template get_offset<set0>(xor_rotate<rf0>(entry0_hash)) << 1;
                const Vector entry0_counter = (entry0_offset + one).gather(set.set, mask);

                mask &= (entry0_counter != Vector(0));

                // for scalar we need to check the mask manually
                if constexpr (not Vector::avx) {
                    if (not mask) {
                        continue;
                    }
                }

                // push hash value onto xor stack
                entry0_hash.scatter(stack, run, mask);
                // increment size of xor stack
                run = run.add_mask(one, mask);

                // compute offset into entries
                const Vector entry1_offset = set.template get_offset<set1>(xor_rotate<rf1>(entry0_hash)) << 1;
                const Vector entry2_offset = set.template get_offset<set2>(xor_rotate<rf2>(entry0_hash)) << 1;

                // remove current hash from set and update it
                const Vector entry1_hash = (entry1_offset.gather(set.set, mask) ^ entry0_hash);
                const Vector entry2_hash = (entry2_offset.gather(set.set, mask) ^ entry0_hash);

                entry1_hash.scatter(set.set, entry1_offset, mask);
                entry2_hash.scatter(set.set, entry2_offset, mask);

                // decrement counter
                const Vector entry1_counter = ((entry1_offset + one).gather(set.set, mask) - one);
                const Vector entry2_counter = ((entry2_offset + one).gather(set.set, mask) - one);

                entry1_counter.scatter(set.set, entry1_offset + one, mask);
                entry2_counter.scatter(set.set, entry2_offset + one, mask);

                const M entry1_mask = mask & (entry1_counter == one);
                const M entry2_mask = mask & (entry2_counter == one);

                if (entry1_mask) {
                    // write offset into xor set where entries with counter 1 to xor queue
                    entry1_hash.scatter(queue.queue, queue1_end, entry1_mask);
                    // increment size of xor queue
                    queue1_end = queue1_end.add_mask(one, entry1_mask);
                }
                if (entry2_mask) {
                    // write offset into xor set where entries with counter 1 to xor queue
                    entry2_hash.scatter(queue.queue, queue2_end, entry2_mask);
                    // increment size of xor queue
                    queue2_end = queue2_end.add_mask(one, entry2_mask);
                }
            }

            queue.template clear<set0>();
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        void fill(Set &set, Queue &queue) {
            while (queue.template size<0>() + queue.template size<1>() + queue.template size<2>() > Vector(0)) {
                process_queue<0, rf0, 1, rf1, 2, rf2>(set, queue);
                process_queue<1, rf1, 2, rf2, 0, rf0>(set, queue);
                process_queue<2, rf2, 0, rf0, 1, rf1>(set, queue);
            }
        }
    };

    template<typename Hasher, typename Vector, typename Addresser>
    struct XorStack<Variant::Fuse, Hasher, Vector, Addresser> {

        using Set = XorSet<Variant::Fuse, Hasher, Vector, Addresser>;
        using Queue = XorQueue<Variant::Fuse, Hasher, Vector, Addresser>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        // list of runs, store offset into the stack
        T *stack;
        Vector begin, end;

        XorStack(const Vector &histogram) {
            T last_offset = 0;
            for (size_t i = 0; i < Vector::n_elements; i++) {
                const size_t n_entries = histogram.elements[i];
                begin.elements[i] = last_offset;
                end.elements[i] = last_offset;
                last_offset += n_entries * 2;
            }

            size_t n_bytes = simd::valign<Vector>(last_offset * sizeof(T));
            stack = simd::valloc<Vector>(n_bytes);
        }

        ~XorStack() {
            free(stack);
        }

        forceinline
        Vector size() const {
            return (end - begin) >> 1;
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        void fill(Set &set, Queue &queue) {
            const Vector segment_size = set.get_segment_size();
            const Vector zero = Vector(0);
            const Vector one = Vector(1);

            // begin, length and end of current queue (needs to be incremented by factor 2)
            Vector queue_begin = queue.begins;
            Vector queue_end = queue.ends;

            // offset into stack
            for (M mask; (mask = queue_begin < queue_end); queue_begin = queue_begin + one) {
                const Vector address = queue_begin.gather(queue.queue, mask);
                const Vector offset = (address + set.begins[0]) << 1;
                const Vector hash = offset.gather(set.set, mask);
                const Vector counter = (offset + one).gather(set.set, mask);
                const Vector segment = xor_segment<Variant::Fuse>(hash) * segment_size;

                const Vector entry0_offset = (segment + set.template get_offset<0>(xor_rotate<rf0>(hash))) << 1;
                const Vector entry1_offset = (segment + set.template get_offset<1>(xor_rotate<rf1>(hash))) << 1;
                const Vector entry2_offset = (segment + set.template get_offset<2>(xor_rotate<rf2>(hash))) << 1;

                const Vector entry0_counter = ((entry0_offset + one).gather(set.set, mask) - one);
                const Vector entry1_counter = ((entry1_offset + one).gather(set.set, mask) - one);
                const Vector entry2_counter = ((entry2_offset + one).gather(set.set, mask) - one);

                mask &= (counter == one);

                // for scalar we need to check the mask manually
                if constexpr (not Vector::avx) {
                    if (not mask) {
                        continue;
                    }
                }

                // push hash value onto xor stack
                hash.scatter(stack, end, mask);
                end = end.add_mask(one, mask);
                // increment size of xor stack
                address.scatter(stack, end, mask);
                end = end.add_mask(one, mask);

                // remove current hash from set and update it
                const Vector entry0_hash = (entry0_offset.gather(set.set, mask) ^ hash);
                const Vector entry1_hash = (entry1_offset.gather(set.set, mask) ^ hash);
                const Vector entry2_hash = (entry2_offset.gather(set.set, mask) ^ hash);

                entry0_hash.scatter(set.set, entry0_offset, mask);
                entry1_hash.scatter(set.set, entry1_offset, mask);
                entry2_hash.scatter(set.set, entry2_offset, mask);

                // write counters back
                entry0_counter.scatter(set.set, entry0_offset + one, mask);
                entry1_counter.scatter(set.set, entry1_offset + one, mask);
                entry2_counter.scatter(set.set, entry2_offset + one, mask);

                const M entry0_mask = mask & (entry0_counter == one);
                const M entry1_mask = mask & (entry1_counter == one);
                const M entry2_mask = mask & (entry2_counter == one);

                if constexpr (Vector::avx) {
                    set.get_address(entry0_offset >> 1).scatter(queue.queue, queue_end, entry0_mask);
                    queue_end = queue_end.add_mask(one, entry0_mask);
                    set.get_address(entry1_offset >> 1).scatter(queue.queue, queue_end, entry1_mask);
                    queue_end = queue_end.add_mask(one, entry1_mask);
                    set.get_address(entry2_offset >> 1).scatter(queue.queue, queue_end, entry2_mask);
                    queue_end = queue_end.add_mask(one, entry2_mask);
                } else {
                    if (entry0_mask) {
                        set.get_address(entry0_offset >> 1).scatter(queue.queue, queue_end, entry0_mask);
                        queue_end = queue_end + one;
                    }
                    if (entry1_mask) {
                        set.get_address(entry1_offset >> 1).scatter(queue.queue, queue_end, entry1_mask);
                        queue_end = queue_end + one;
                    }
                    if (entry2_mask) {
                        set.get_address(entry2_offset >> 1).scatter(queue.queue, queue_end, entry2_mask);
                        queue_end = queue_end + one;
                    }
                }
            }
        }
    };

} // filters::nxor
