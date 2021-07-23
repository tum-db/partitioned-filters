#pragma once

#include <bloom/bloom_parameter.hpp>
#include <partition/partition_set.hpp>
#include <hash/hasher.hpp>
#include <bloom/mask/bloom_mask.hpp>
#include <hash/seed.hpp>
#include "bloom_construct_base.hpp"

namespace filters::bloom {

    template<BlockSize blockSize, Groups groups, size_t k, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomConstructor<Variant::Naive, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd,
            Container, Hasher> {

        static_assert(static_cast<size_t>(blockSize) == static_cast<size_t>(registerSize),
                "BlockSize and RegisterSize must be equal for Naive Bloom Filter!");

        static constexpr size_t remaining_hash_bits = static_cast<size_t>(registerSize) - Container::address_bits;

        using Vector = simd::Vector<registerSize, _simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        template<size_t remaining_k, size_t _remaining_address_hash_bits, size_t _remaining_block_hash_bits, size_t _n_address_rehash, size_t _n_block_rehash>
        struct Helper {

            using Mask = BloomMask<Variant::Sectorized, Vectorization::Horizontal, blockSize, groups, 1, registerSize,
                    _simd, 1, _remaining_block_hash_bits, _n_block_rehash, Hasher>;

            static constexpr size_t remaining_address_hash_bits = (_remaining_address_hash_bits <
                                                                   Container::address_bits)
                                                                  ? Hasher::hash_bits
                                                                  : _remaining_address_hash_bits;
            static constexpr size_t n_address_rehash =
                    _n_address_rehash + (_remaining_address_hash_bits < Container::address_bits);
            static constexpr size_t remaining_block_hash_bits = Mask::Helper::remaining_hash_bits;
            static constexpr size_t n_block_rehash = Mask::Helper::n_rehash;

            using Recursion = Helper<remaining_k - 1, remaining_address_hash_bits - Container::address_bits,
                    remaining_block_hash_bits, n_address_rehash, n_block_rehash>;

            forceinline
            static void
            help(Container &container, const size_t i, const Vector &values, Vector &address_hash, Vector &block_hash,
                 const M mask) {
                if constexpr (_remaining_address_hash_bits < Container::address_bits) {
                    address_hash = Hasher::template rehash<_n_address_rehash>(address_hash, values);
                }

                Vector block_offset = container.compute_offset_horizontal(i,
                        simd::extractBits(address_hash, Hasher::hash_bits - remaining_address_hash_bits,
                                Container::address_bits));

                Mask::generate_mask(values, block_hash, container.data(), block_offset, mask);

                if constexpr (remaining_k > 1) {
                    Recursion::help(container, i, values, address_hash, block_hash, mask);
                }
            }
        };

        using H = Helper<k, Hasher::hash_bits, Hasher::hash_bits, 0, 0>;

        const Vector seed = hash::seed<Vector, Vector::n_elements>();

        void insert(Container &container, const Vector &values, const size_t index) {
            insert(container, values, Vector::mask(1), index);
        }

        forceinline
        void insert(Container &container, const Vector &values, const M mask, const size_t index) {
            Vector address_hash = Hasher::hash(values);
            Vector block_hash = Hasher::hash(values ^ seed);

            H::help(container, index, values, address_hash, block_hash, mask);
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
            for (size_t i = 0; i < partitions.n_partitions; i += H::Mask::n_lanes) {
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
            for (size_t i = 0; i < partitions.n_partitions; i += H::Mask::n_lanes) {
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