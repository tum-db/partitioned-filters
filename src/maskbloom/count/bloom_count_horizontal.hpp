#pragma once

#include "maskbloom/maskbloom_parameter.hpp"
#include "maskbloom/mask/maskbloom_mask.hpp"
#include "simd/vector.hpp"

namespace filters::maskbloom {

    template <Variant variant, parameter::RegisterSize registerSize, parameter::SIMD _simd, typename Container, typename Hasher>
    struct MaskBloomCounter {

        static constexpr size_t remaining_hash_bits = static_cast<size_t>(registerSize) - Container::address_bits;

        using Mask = MaskBloomMask<variant, registerSize, _simd, remaining_hash_bits, Hasher>;
        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;
        using T = typename Vector::T;

        forceinline

        M contains(const Container& container, const Vector& values, const size_t index) const {
            return contains(container, values, Vector::mask(1), index);
        }

        forceinline

        M contains(const Container& container, const Vector& values, const M mask, const size_t index) const {
            Vector hash = Hasher::hash(values);

            const Vector block_offset = container.compute_offset_vertical(index, simd::extractBits(hash, 0, Container::address_bits));

            if constexpr (Vector::avx) {
                return Mask::check_mask(values, hash, container.data(), block_offset, mask);
            } else {
                return Mask::check_mask(values, hash, container.data() + block_offset.vector, Vector(0), mask);
            }
        }

        template <typename PartitionSet>
        forceinline
        size_t count(const Container& container, const PartitionSet& partitions) const {
            size_t counter = 0;

            for (size_t i = 0; i < partitions.n_partitions; i += 1) {
                const typename Vector::T* begin = partitions.values + partitions.offsets[i];
                const typename Vector::T* end = begin + partitions.histogram[i];


                for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                    const Vector values = Vector::load(begin);
                    counter += Vector::popcount_mask(contains(container, values, i));
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

        template <typename PartitionSet, typename TaskQueue>


        forceinline
        void count(const Container& container, const PartitionSet& partitions, std::atomic<size_t>& counter, TaskQueue& queue) const {
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
                        const T* begin = partitions.values + partitions.offsets[i] +
                            simd::valign<Vector>((length / n_tasks + 1) * j);
                        const T* end = partitions.values + partitions.offsets[i] +
                            std::min(simd::valign<Vector>((length / n_tasks + 1) * (j + 1)), length);
                        size_t local_counter = 0;

                        for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                            const Vector values = Vector::load(begin);
                            local_counter += Vector::popcount_mask(contains(container, values, i));
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
