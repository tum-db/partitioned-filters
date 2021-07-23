#pragma once

#include <bloom/bloom_parameter.hpp>

namespace filters::bloom {

    template<Variant variant, BlockSize blockSize, Groups groups, size_t k, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomCounter<variant, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd, Container,
            Hasher> {

        static constexpr size_t remaining_hash_bits = static_cast<size_t>(registerSize) - Container::address_bits;
        static constexpr size_t shift_factor = math::const_log2(
                static_cast<size_t>(blockSize) / static_cast<size_t>(registerSize));

        using Mask = BloomMask<variant, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd, k,
                remaining_hash_bits, 0, Hasher>;
        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;
        using T = typename Vector::T;

        forceinline
        M contains(const Container &container, const Vector &values, const size_t index) const {
            return contains(container, values, Vector::mask(1), index);
        }

        forceinline
        M contains(const Container &container, const Vector &values, const M mask, const size_t index) const {
            Vector hash = Hasher::hash(values);

            const Vector block_offset =
                    container.compute_offset_vertical(index, simd::extractBits(hash, 0, Container::address_bits))
                            << shift_factor;

            if constexpr (Vector::avx) {
                return Mask::check_mask(values, hash, container.data(), block_offset, mask);
            } else {
                return Mask::check_mask(values, hash, container.data() + block_offset.vector, Vector(0), mask);
            }
        }

        template<typename PartitionSet>
        forceinline
        size_t count(const Container &container, const PartitionSet &partitions) const {
            size_t counter = 0;

            for (size_t i = 0; i < partitions.n_partitions; i += 1) {
                const typename Vector::T *begin = partitions.values + partitions.offsets[i];
                const typename Vector::T *end = begin + partitions.histogram[i];

                if constexpr (Vector::avx and Vector::registerSize == parameter::RegisterSize::_32bit and
                              (variant == Variant::Blocked and
                               static_cast<size_t>(blockSize) > static_cast<size_t>(BlockSize::_64bit))) {
                    Vector index(0);
                    for (size_t l = 0; l < Vector::n_elements; l++) {
                        index.elements[l] = l;
                    }

                    if (begin + Vector::n_elements <= end) {
                        Vector values = Vector::load(begin);
                        for (; begin + 2 * Vector::n_elements <= end;) {
                            counter += Vector::popcount_mask(contains(container, values, i));
                            begin += Vector::n_elements;
                            values = index.gather(begin, Vector::mask(1));
                        }
                        counter += Vector::popcount_mask(contains(container, values, i));
                        begin += Vector::n_elements;
                    }
                } else {
                    for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                        const Vector values = Vector::load(begin);
                        counter += Vector::popcount_mask(contains(container, values, i));
                    }
                }

                if constexpr (Vector::avx) {
                    if (begin < end) {
                        M mask = (1 << (end - begin)) - 1;
                        const Vector values = Vector::load(begin);
                        counter += Vector::popcount_mask(contains(container, values, mask, i));
                    }
                }
            }

            return counter;
        }

        template<typename PartitionSet, typename TaskQueue>
        forceinline
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

                        if constexpr (Vector::avx and Vector::registerSize == parameter::RegisterSize::_32bit and
                                      (variant == Variant::Blocked and
                                       static_cast<size_t>(blockSize) > static_cast<size_t>(BlockSize::_64bit))) {
                            Vector index(0);
                            for (size_t l = 0; l < Vector::n_elements; l++) {
                                index.elements[l] = l;
                            }

                            if (begin + Vector::n_elements <= end) {
                                Vector values = Vector::load(begin);
                                for (; begin + 2 * Vector::n_elements <= end;) {
                                    counter += Vector::popcount_mask(contains(container, values, i));
                                    begin += Vector::n_elements;
                                    values = index.gather(begin, Vector::mask(1));
                                }
                                counter += Vector::popcount_mask(contains(container, values, i));
                                begin += Vector::n_elements;
                            }
                        } else {
                            for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                                const Vector values = Vector::load(begin);
                                counter += Vector::popcount_mask(contains(container, values, i));
                            }
                        }

                        if constexpr (Vector::avx) {
                            if (begin < end) {
                                M mask = (1 << (end - begin)) - 1;
                                const Vector values = Vector::load(begin);
                                local_counter += Vector::popcount_mask(contains(container, values, mask, i));
                            }
                        }

                        counter += local_counter;
                    });
                }
            }
        }
    };

} // filters::bloom