#pragma once

#include <bloom/bloom_parameter.hpp>
#include <partition/partition_set.hpp>
#include <hash/hasher.hpp>
#include <bloom/mask/bloom_mask.hpp>
#include <hash/seed.hpp>
#include "bloom_count_base.hpp"

namespace filters::bloom {

    template<BlockSize blockSize, Groups groups, size_t k, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomCounter<Variant::Naive, Vectorization::Horizontal, blockSize, groups, k, registerSize, _simd, Container,
            Hasher> {

        static_assert(static_cast<size_t>(blockSize) == static_cast<size_t>(registerSize),
                "BlockSize and RegisterSize must be equal for Naive Bloom Filter!");

        static constexpr size_t remaining_hash_bits = static_cast<size_t>(registerSize) - Container::address_bits;

        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;
        using T = typename Vector::T;

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
            static M help(const Container &container, const size_t i, const Vector &values, Vector &address_hash,
                          Vector &block_hash, const M mask) {
                if constexpr (_remaining_address_hash_bits < Container::address_bits) {
                    address_hash = Hasher::template rehash<_n_address_rehash>(address_hash, values);
                }

                Vector block_offset = container.compute_offset_vertical(i,
                        simd::extractBits(address_hash, Hasher::hash_bits - remaining_address_hash_bits,
                                Container::address_bits));

                const M found = Mask::check_mask(values, block_hash, container.data(), block_offset, mask);

                if constexpr (remaining_k > 1) {
                    if constexpr (not Vector::avx) {
                        if (found == 0) {
                            return 0;
                        }

                        return found & Recursion::help(container, i, values, address_hash, block_hash, mask);
                    } else {
                        return Recursion::help(container, i, values, address_hash, block_hash, found & mask);
                    }
                } else {
                    return found & mask;
                }
            }
        };

        using H = Helper<k, Hasher::hash_bits, Hasher::hash_bits, 0, 0>;

        const Vector seed = hash::seed<Vector, Vector::n_elements>();

        forceinline
        M contains(const Container &container, const Vector &values, const size_t index) const {
            return contains(container, values, Vector::mask(1), index);
        }

        forceinline
        M contains(const Container &container, const Vector &values, const M mask, const size_t index) const {
            Vector address_hash = Hasher::hash(values);
            Vector block_hash = Hasher::hash(values ^ seed);

            return H::help(container, index, values, address_hash, block_hash, mask);
        }

        template<typename PartitionSet>
        forceinline
        size_t count(const Container &container, const PartitionSet &partitions) const {
            size_t counter = 0;

            for (size_t i = 0; i < partitions.n_partitions; i += 1) {
                const typename Vector::T *begin = partitions.values + partitions.offsets[i];
                const typename Vector::T *end = begin + partitions.histogram[i];

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

                        for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                            const Vector values = Vector::load(begin);
                            local_counter += Vector::popcount_mask(contains(container, values, i));
                        }

                        if constexpr (Vector::avx) {
                            if (begin < end) {
                                M mask = (1ull << (end - begin)) - 1;
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