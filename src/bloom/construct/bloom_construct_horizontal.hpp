#pragma once

#include <bloom/bloom_parameter.hpp>
#include <partition/partition_set.hpp>
#include <hash/hasher.hpp>
#include <bloom/mask/bloom_mask.hpp>

namespace filters::bloom {

    template<Variant variant, BlockSize blockSize, Groups groups, size_t k, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomConstructor<variant, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd, Container,
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
        void insert(Container &container, const Vector &values, const size_t index) {
            insert(container, values, Vector::mask(1), index);
        }

        forceinline
        void insert(Container &container, const Vector &values, const M mask, const size_t index) {
            Vector hash = Hasher::hash(values);
            Vector block_offset =
                    container.compute_offset_horizontal(index, simd::extractBits(hash, 0, Container::address_bits))
                            << shift_factor;
            if constexpr (Vector::avx) {
                Mask::generate_mask(values, hash, container.data(), block_offset, mask);
            } else {
                Mask::generate_mask(values, hash, container.data() + block_offset.vector, Vector(0), mask);
            }
        }

        forceinline
        void construct_partition(Container &container, const T *values, const size_t length, const size_t index) {
            static_assert(not Vector::avx);

            for (Vector begin = Vector(0); begin < Vector(length); begin = begin + Vector(1)) {
                const Vector value = begin.gather(values, 1);
                insert(container, value, 1, index);
            }
        }

        template<typename PartitionSet>
        forceinline
        void construct(Container &container, const PartitionSet &partitions) {
            for (size_t i = 0; i < partitions.n_partitions; i += Mask::n_lanes) {
                Vector begin = Vector::load(partitions.offsets + i);
                const Vector end = begin + Vector::load(partitions.histogram + i);

                for (M mask; (mask = begin < end); begin = begin + Vector(1)) {
                    const Vector values = begin.gather(partitions.values, mask);
                    insert(container, values, mask, i);
                }
            }
        }

        template<typename PartitionSet, typename TaskQueue>
        forceinline
        void construct(Container &container, const PartitionSet &partitions, TaskQueue &queue) {
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