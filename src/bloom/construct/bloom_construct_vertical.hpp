#pragma once

#include <bloom/bloom_parameter.hpp>
#include <partition/partition_set.hpp>
#include <hash/hasher.hpp>
#include <bloom/mask/bloom_mask.hpp>

namespace filters::bloom {

    template<Variant variant, BlockSize blockSize, Groups groups, size_t k, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomConstructor<variant, Vectorization::Vertical, blockSize, groups, k, registerSize, _simd, Container,
            Hasher> {

        static constexpr size_t remaining_hash_bits = 0;
        static constexpr size_t shift_factor = math::const_log2(
                static_cast<size_t>(blockSize) / static_cast<size_t>(registerSize));

        using Mask = BloomMask<variant, Vectorization::Vertical, blockSize, groups, k, registerSize, _simd, k,
                remaining_hash_bits, 0, Hasher>;
        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;
        using T = typename Vector::T;

        static constexpr size_t n_lanes = Mask::n_lanes;
        static constexpr size_t n_elements = Vector::n_elements;
        static constexpr size_t n_elements_per_lane = n_elements / n_lanes;

        Vector base_offset;
        Vector seed = hash::seed<Vector, n_lanes>();
        Vector permute_masks[n_elements / n_lanes];

        BloomConstructor() {
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
        }

        forceinline unroll_loops
        void insert(Container &container, const Vector &values, const size_t index) {
            const Vector hash = Hasher::hash(values);
            const Vector block_offset =
                    container.compute_offset_vertical(index, simd::extractBits(hash, 0, Container::address_bits))
                            << shift_factor;

            for (size_t j = 0; j < n_elements_per_lane; j++) {
                const Vector v = values.permute(permute_masks[j]);
                Vector h = hash.permute(permute_masks[j]) ^seed;
                const Vector o = block_offset.permute(permute_masks[j]) + base_offset;

                Mask::generate_mask(v, h, container.data(), o, Vector::mask(1));
            }
        }

        forceinline unroll_loops
        void insert(Container &container, const Vector &values, M mask, const size_t index) {
            const Vector hash = Hasher::hash(values);
            const Vector block_offset =
                    container.compute_offset_vertical(index, simd::extractBits(hash, 0, Container::address_bits))
                            << shift_factor;

            for (size_t j = 0; j < n_elements_per_lane && mask; j++) {
                M m = Vector::mask(0);
                for (size_t l = 0; l < n_lanes && mask; l++) {
                    m |= ((1ull << n_elements_per_lane) - 1) * (mask & 0b1) << (l * n_elements_per_lane);
                    mask >>= 1;
                }

                const Vector v = values.permute(permute_masks[j]);
                Vector h = hash.permute(permute_masks[j]) ^seed;
                const Vector o = block_offset.permute(permute_masks[j]) + base_offset;

                Mask::generate_mask(v, h, container.data(), o, m);
            }
        }

        forceinline
        void construct_partition(Container &container, const T *values, const size_t length, const size_t index) {
            const T *begin = values;
            const T *end = begin + length;

            for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                const Vector values = Vector::load(begin);
                insert(container, values, index);
            }

            if constexpr (Vector::avx) {
                if (begin < end) {
                    const M mask = (1 << (end - begin)) - 1;
                    const Vector values = Vector::load(begin);

                    insert(container, values, mask, index);
                }
            }
        }

        template<typename PartitionSet>
        forceinline
        void construct(Container &container, const PartitionSet &partitions) {
            for (size_t i = 0; i < partitions.n_partitions; i++) {
                construct_partition(container, partitions.values + partitions.offsets[i], partitions.histogram[i], i);
            }
        }

        template<typename PartitionSet, typename TaskQueue>
        forceinline
        void construct(Container &container, const PartitionSet &partitions, TaskQueue &queue) {
            for (size_t i = 0; i < partitions.n_partitions; i++) {
                queue.add_task([this, &container, &partitions, i](size_t) {
                    construct_partition(container, partitions.values + partitions.offsets[i], partitions.histogram[i],
                            i);
                });
            }
        }
    };

} // filters::bloom