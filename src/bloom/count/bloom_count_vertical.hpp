#pragma once

#include <bloom/bloom_parameter.hpp>

namespace filters::bloom {

    template<Variant variant, BlockSize blockSize, Groups groups, size_t k, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomCounter<variant, Vectorization::Vertical, blockSize, groups, k, registerSize, _simd, Container,
            Hasher> {

        static constexpr size_t remaining_hash_bits = 0;
        static constexpr size_t shift_factor = math::const_log2(
                static_cast<size_t>(blockSize) / static_cast<size_t>(registerSize));

        using Mask = BloomMask<variant, Vectorization::Vertical, blockSize, groups, k, registerSize, _simd, k,
                remaining_hash_bits, 0, Hasher>;
        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;
        using T = typename Vector::T;

        static_assert(Vector::avx, "Vertical variants require AVX!");

        static constexpr size_t n_lanes = Mask::n_lanes;
        static constexpr size_t n_elements = Vector::n_elements;
        static constexpr size_t n_elements_per_lane = n_elements / n_lanes;

        Vector base_offset{0};
        Vector seed = hash::seed<Vector, n_lanes>();
        Vector permute_masks[n_elements / n_lanes];
        uint32_t pext_mask = 1;

        BloomCounter() {
            for (size_t i = 0; i < n_lanes; i++) {
                for (size_t j = 0; j < n_elements_per_lane; j++) {
                    base_offset.elements[i * n_elements_per_lane + j] = j;
                }
            }

            for (size_t i = 0; i < n_elements / n_lanes; i++) {
                for (size_t j = 0; j < n_lanes; j++) {
                    for (size_t l = 0; l < n_elements_per_lane; l++) {
                        permute_masks[i].elements[j * n_elements_per_lane + l] = i * n_lanes + j;
                    }
                }
            }

            for (size_t i = 0; i < n_lanes; i++) {
                pext_mask |= (pext_mask << n_elements_per_lane);
            }
        }

        forceinline unroll_loops
        M _contains(const Container &container, const Vector &values, const Vector &hash, const Vector &block_offset,
                    const M m, const size_t j) const {
            const Vector v = values.permute(permute_masks[j]);
            Vector h = hash.permute(permute_masks[j]) ^seed;
            const Vector o = block_offset.permute(permute_masks[j]) + base_offset;

            M equal = Mask::check_mask(v, h, container.data(), o, m);

            if constexpr (n_lanes == 1) {
                return (equal == Vector::mask(1));
            } else {
                for (size_t l = 1; l < n_elements_per_lane; l <<= 1) {
                    equal &= (equal >> l);
                }
                return equal;
            }
        }

        forceinline unroll_loops
        M contains(const Container &container, const Vector &values, const size_t index) const {
            const Vector hash = Hasher::hash(values);
            const Vector block_offset =
                    container.compute_offset_vertical(index, simd::extractBits(hash, 0, Container::address_bits))
                            << shift_factor;

            M found = Vector::mask(0);
            for (size_t j = 0; j < n_elements_per_lane; j++) {
                const M m = _contains(container, values, hash, block_offset, Vector::mask(1), j);

                if constexpr (n_lanes == 1) {
                    found |= (m << j);
                } else {
                    found |= (simd::pext32(m, pext_mask) << (j * n_lanes));
                }
            }
            return found;
        }

        forceinline unroll_loops
        M contains(const Container &container, const Vector &values, M mask, const size_t index) const {
            const Vector hash = Hasher::hash(values);
            const Vector block_offset =
                    container.compute_offset_vertical(index, simd::extractBits(hash, 0, Container::address_bits))
                            << shift_factor;

            M found = Vector::mask(0);
            for (size_t j = 0; j < n_elements_per_lane && mask; j++) {
                M m = Vector::mask(0);
                for (size_t l = 0; l < n_lanes && mask; l++) {
                    m |= ((1ull << n_elements_per_lane) - 1) * (mask & 0b1) << (l * n_elements_per_lane);
                    mask >>= 1;
                }

                m = _contains(container, values, hash, block_offset, m, j);

                if constexpr (n_lanes == 1) {
                    found |= (m << j);
                } else {
                    found |= (simd::pext32(m, pext_mask) << (j * n_lanes));
                }
            }
            return found;
        }

        template<typename PartitionSet>
        forceinline unroll_loops
        size_t count(const Container &container, const PartitionSet &partitions) const {
            size_t counter = 0;

            for (size_t i = 0; i < partitions.n_partitions; i++) {
                const T *begin = partitions.values + partitions.offsets[i];
                const T *end = begin + partitions.histogram[i];

                for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                    const Vector values = Vector::load(begin);
                    const Vector hash = Hasher::hash(values);
                    const Vector block_offset =
                            container.compute_offset_vertical(i, simd::extractBits(hash, 0, Container::address_bits))
                                    << shift_factor;

                    for (size_t j = 0; j < n_elements_per_lane; j++) {
                        const M found = _contains(container, values, hash, block_offset, Vector::mask(1), j);

                        if constexpr (n_lanes == 1) {
                            counter += found;
                        } else {
                            counter += Vector::popcount_mask(found & pext_mask);
                        }
                    }
                }

                if (begin < end) {
                    M mask = (1 << (end - begin)) - 1;
                    const Vector values = Vector::load(begin);
                    const Vector hash = Hasher::hash(values);
                    const Vector block_offset =
                            container.compute_offset_vertical(i, simd::extractBits(hash, 0, Container::address_bits))
                                    << shift_factor;

                    for (size_t j = 0; j < n_elements_per_lane && mask; j++) {
                        M m = Vector::mask(0);
                        for (size_t l = 0; l < n_lanes && mask; l++) {
                            m |= ((1ull << n_elements_per_lane) - 1) * (mask & 0b1) << (l * n_elements_per_lane);
                            mask >>= 1;
                        }

                        const M found = _contains(container, values, hash, block_offset, m, j);

                        if constexpr (n_lanes == 1) {
                            counter += found;
                        } else {
                            counter += Vector::popcount_mask(found & pext_mask);
                        }
                    }
                }
            }

            return counter;
        }

        template<typename PartitionSet, typename TaskQueue>
        forceinline unroll_loops
        void count(const Container &container, const PartitionSet &partitions, std::atomic<size_t> &counter,
                   TaskQueue &queue) const {
            size_t remaining_tasks = queue.get_n_tasks_per_level();

            for (size_t i = 0; i < partitions.n_partitions; i++) {
                // add at least one task
                size_t n_tasks = 1;
                if (remaining_tasks > 0) {
                    n_tasks = std::max(remaining_tasks / (partitions.n_partitions - i), static_cast<size_t>(1));
                    remaining_tasks -= n_tasks;
                }

                for (size_t j = 0; j < n_tasks; j++) {
                    queue.add_task([this, &container, &counter, &partitions, i, j, n_tasks](size_t) {
                        const size_t length = partitions.histogram[i];
                        const T *begin = partitions.values + partitions.offsets[i] +
                                         simd::valign<Vector>((length / n_tasks + 1) * j);
                        const T *end = partitions.values + partitions.offsets[i] +
                                       std::min(simd::valign<Vector>((length / n_tasks + 1) * (j + 1)), length);
                        size_t local_counter = 0;

                        for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                            const Vector values = Vector::load(begin);
                            const Vector hash = Hasher::hash(values);
                            const Vector block_offset = container.compute_offset_vertical(i,
                                    simd::extractBits(hash, 0, Container::address_bits)) << shift_factor;

                            for (size_t l = 0; l < n_elements_per_lane; l++) {
                                const M found = _contains(container, values, hash, block_offset, Vector::mask(1), l);

                                if constexpr (n_lanes == 1) {
                                    counter += found;
                                } else {
                                    counter += Vector::popcount_mask(found & pext_mask);
                                }
                            }
                        }

                        if (begin < end) {
                            M mask = (1 << (end - begin)) - 1;
                            const Vector values = Vector::load(begin);
                            const Vector hash = Hasher::hash(values);
                            const Vector block_offset = container.compute_offset_vertical(i,
                                    simd::extractBits(hash, 0, Container::address_bits)) << shift_factor;

                            for (size_t l = 0; l < n_elements_per_lane && mask; l++) {
                                M m = Vector::mask(0);
                                for (size_t n = 0; n < n_lanes && mask; n++) {
                                    m |= ((1ull << n_elements_per_lane) - 1) * (mask & 0b1)
                                            << (n * n_elements_per_lane);
                                    mask >>= 1;
                                }

                                const M found = _contains(container, values, hash, block_offset, m, l);

                                if constexpr (n_lanes == 1) {
                                    counter += found;
                                } else {
                                    counter += Vector::popcount_mask(found & pext_mask);
                                }
                            }
                        }

                        counter += local_counter;
                    });
                }
            }
        }
    };

} // filters::bloom