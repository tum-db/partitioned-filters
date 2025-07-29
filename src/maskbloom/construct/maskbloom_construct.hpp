#pragma once

#include <maskbloom/mask/maskbloom_mask.hpp>
#include "maskbloom/maskbloom_parameter.hpp"

namespace filters::maskbloom {

    template <Variant variant, parameter::RegisterSize registerSize, parameter::SIMD _simd, typename Container, typename Hasher>
    struct MaskBloomConstructor {

        static constexpr size_t remaining_hash_bits = static_cast<size_t>(registerSize) - Container::address_bits;

        using Mask = MaskBloomMask<variant, registerSize, _simd, remaining_hash_bits, Hasher>;
        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;
        using T = typename Vector::T;

        forceinline

        void insert(Container& container, const Vector& values, const size_t index) {
            insert(container, values, Vector::mask(1), index);
        }

        forceinline

        void insert(Container& container, const Vector& values, const M mask, const size_t index) {
            Vector hash = Hasher::hash(values);
            Vector block_offset = container.compute_offset_horizontal(index, simd::extractBits(hash, 0, Container::address_bits));
            if constexpr (Vector::avx) {
                Mask::generate_mask(values, hash, container.data(), block_offset, mask);
            } else {
                Mask::generate_mask(values, hash, container.data() + block_offset.vector, Vector(0), mask);
            }
        }

        forceinline

        void construct_partition(Container& container, const T* values, const size_t length, const size_t index) {
            static_assert(not Vector::avx);

            for (Vector begin = Vector(0); begin < Vector(length); begin = begin + Vector(1)) {
                const Vector value = begin.gather(values, 1);
                insert(container, value, 1, index);
            }
        }

        template <typename PartitionSet>
        forceinline
        void construct(Container& container, const PartitionSet& partitions) {
            for (size_t i = 0; i < partitions.n_partitions; i += Mask::n_lanes) {
                Vector begin = Vector::load(partitions.offsets + i);
                const Vector end = begin + Vector::load(partitions.histogram + i);

                for (M mask; (mask = begin < end); begin = begin + Vector(1)) {
                    const Vector values = begin.gather(partitions.values, mask);
                    insert(container, values, mask, i);
                }
            }
        }

        template <typename PartitionSet, typename TaskQueue>
        forceinline
        void construct(Container& container, const PartitionSet& partitions, TaskQueue& queue) {
            for (size_t i = 0; i < partitions.n_partitions; i += Mask::n_lanes) {
                queue.add_task([this, &container, &partitions, i](size_t) {
                    Vector begin = Vector::load(partitions.offsets + i);
                    const Vector end = begin + Vector::load(partitions.histogram + i);

                    for (M mask; (mask = begin < end); begin = begin + Vector(1)) {
                        const Vector values = begin.gather(partitions.values, mask);
                        insert(container, values, mask, i);
                    }
                });
            }
        }
    };

} // filters::bloom
