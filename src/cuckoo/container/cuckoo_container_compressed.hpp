#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <partition/partition_set.hpp>
#include <simd/helper.hpp>
#include <cuckoo/cuckoo_parameter.hpp>
#include <math/math.hpp>
#include <parameter/parameter.hpp>
#include <random/random.hpp>
#include <simd/vector_div.hpp>
#include "cuckoo_container_base.hpp"
#include "cuckoo_helper.hpp"

namespace filters::cuckoo {

    template<size_t associativity, size_t buckets_per_block, size_t ota_size, size_t k, parameter::Partitioning partitioning, parameter::RegisterSize registerSize, parameter::SIMD simd, parameter::AddressingMode addressingMode, parameter::HashingMode hashingMode>
    struct CuckooContainer<false, Variant::Morton, associativity, buckets_per_block, ota_size, k, partitioning,
            registerSize, simd, addressingMode, hashingMode> {

        using Vector = simd::Vector<registerSize, simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;
        using Addresser = addresser::Addresser<addressingMode, Vector>;
        using Hasher = hash::Hasher<hashingMode, Vector, 0>;

        static constexpr size_t block_size = 512;
        static constexpr size_t slots_per_bucket = associativity;
        static constexpr size_t counter_size = math::get_number_of_bits(slots_per_bucket + 1);
        static constexpr size_t slots_per_block = (512 - buckets_per_block * counter_size - ota_size) / k;

        static constexpr size_t counters_size_in_bits = buckets_per_block * counter_size;
        static constexpr size_t counters_offset = 0;
        static constexpr size_t ota_offset = counters_size_in_bits;

        static constexpr size_t bits_per_bucket = k * slots_per_bucket;
        static constexpr bool optimal_fingerprint_size = (k == 8) or (k == 16) or (k == 32);
        static constexpr bool avx = Vector::avx;
        static constexpr bool ota = ota_size > 0;
        static constexpr size_t slots_per_bucket_log = math::const_log2(slots_per_bucket);
        static constexpr size_t register_size = static_cast<size_t>(Vector::registerSize);
        static constexpr size_t slots_to_load = (math::is_power_of_two(k) or avx)
                                                ? std::min(associativity, register_size / k)
                                                : std::min(associativity, (register_size - 8) / k);
        static constexpr size_t counter_to_load = (math::is_power_of_two(counter_size) or avx)
                                                  ? std::min(buckets_per_block, register_size / counter_size)
                                                  : std::min(buckets_per_block, (register_size - 8) / counter_size);

        using Fingerprint = std::conditional_t<k <= 8, uint8_t,
                std::conditional_t<k <= 16, uint16_t, std::conditional_t<k <= 32, uint32_t, uint64_t>>>;
        using PartialBucket = std::conditional_t<slots_to_load * k <= 8, uint8_t,
                std::conditional_t<slots_to_load * k <= 16, uint16_t,
                        std::conditional_t<slots_to_load * k <= 32, uint32_t, uint64_t>>>;
        static constexpr PartialBucket fingerprint_mask = (1ull << k) - 1;
        static constexpr PartialBucket bucket_mask = (slots_to_load * k == sizeof(PartialBucket) * 8)
                                                     ? -1
                                                     : (1ull << (slots_to_load * k)) - 1;

        static constexpr uint64_t pext_mask = (k == 8 and register_size == 32)
                                              ? 0x1111111111111111
                                              : ((k == 8 and register_size == 64)
                                                 ? 0x0101010101010101
                                                 : ((k == 16 and register_size == 32)
                                                    ? 0x55555555
                                                    : ((k == 16 and register_size == 64)
                                                       ? 0x11111111
                                                       : 0)));

        Addresser addresser;
        uint8_t *filter;
        size_t n_bytes{0};
        T *offsets;

        CuckooContainer() : filter(nullptr), offsets(nullptr) {
        }

        explicit CuckooContainer(CuckooContainer &&other) : addresser(std::move(other.addresser)), filter(other.filter),
                                                            n_bytes(other.n_bytes), offsets(other.offsets) {
            other.filter = nullptr;
            other.offsets = nullptr;
        }

        CuckooContainer &operator=(CuckooContainer &&other) noexcept {
            if (this != &other) {
                if (offsets) {
                    free(offsets);
                }
                if (filter) {
                    simd::vfree(filter, n_bytes, true);
                }

                addresser = std::move(other.addresser);
                filter = other.filter;
                n_bytes = other.n_bytes;
                offsets = other.offsets;

                other.filter = nullptr;
                other.offsets = nullptr;
            }
            return *this;
        }

        CuckooContainer(size_t s, size_t n_partitions, const T *histogram) {
            rng.reset();
            // number of buckets per partition
            T *h = new T[n_partitions];

            for (size_t i = 0; i < n_partitions; i++) {
                h[i] = math::align_number(histogram[i] * k * s * buckets_per_block / 100, block_size) / block_size;
            }

            addresser = std::move(Addresser(h, n_partitions));
            delete[](h);

            T last_offset = 0;
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                offsets = simd::valloc<Vector>(simd::valign<Vector>(n_partitions * sizeof(T)));
                for (size_t i = 0; i < n_partitions; i++) {
                    size_t n_buckets = this->addresser.get_size(i);
                    offsets[i] = last_offset;
                    last_offset +=
                            (math::align_number(n_buckets, buckets_per_block) / buckets_per_block + 1) * block_size / 8;
                }
            } else {
                size_t n_buckets = this->addresser.get_size(0);
                offsets = nullptr;
                last_offset +=
                        (math::align_number(n_buckets, buckets_per_block) / buckets_per_block + 1) * block_size / 8;
            }

            n_bytes = simd::valign<Vector>(last_offset);
            if (n_bytes > 0) {
                filter = reinterpret_cast<uint8_t *>(simd::valloc<Vector>(n_bytes, 0, true));
            }
        }

        CuckooContainer(uint8_t *filter, const size_t length, const size_t s) : filter(filter), offsets(nullptr) {
            static_assert(partitioning == parameter::Partitioning::Disabled);

            T histogram = math::align_number(length * k * s * buckets_per_block / 100, block_size) / block_size;
            addresser = std::move(Addresser(&histogram, 1));
        }

        ~CuckooContainer() {
            if (offsets) {
                free(offsets);
            }
            if (filter and n_bytes > 0) {
                simd::vfree(filter, n_bytes, true);
            }
        }

        forceinline
        Vector offset_vertical(const size_t index) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                return Vector(offsets[index]);
            } else {
                return Vector(0);
            }
        }

        forceinline
        Vector offset_horizontal(const size_t index) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                return Vector::load(offsets + index);
            } else {
                return Vector(0);
            }
        }

        forceinline
        Vector get_slot_count(const Vector &bucket_index, const Vector &block_offset_in_bits, const M mask) const {
            const Vector bit_address = block_offset_in_bits + bucket_index * Vector(counter_size);
            return simd::_gatheru_bits<counter_size, true>(bit_address, filter, mask);
        }

        forceinline
        void set_slot_count(const Vector &bucket_index, const Vector &block_offset_in_bits, const Vector &counter,
                            const M mask) {
            const Vector bit_address = block_offset_in_bits + bucket_index * Vector(counter_size);
            simd::_scatteru_bits<counter_size, true>(bit_address, filter, counter, mask);
        }

        forceinline
        M get_ota(const Vector &bucket_index, const Vector &block_offset_in_bits, const M mask) const {
            if constexpr (ota) {
                const Vector bit_address =
                        block_offset_in_bits + Vector(ota_offset) + simd::mod<ota_size>(bucket_index);
                return simd::_gatheru_bits<1, true>(bit_address, filter, mask) == Vector(1);
            } else {
                return Vector::mask(1);
            }
        }

        forceinline
        void set_ota(const Vector &bucket_index, const Vector &block_offset_in_bits, const M mask) {
            if constexpr (ota) {
                const Vector bit_address =
                        block_offset_in_bits + Vector(ota_offset) + simd::mod<ota_size>(bucket_index);
                simd::_scatteru_bits<1, true>(bit_address, filter, Vector(1), mask);
            }
        }

        forceinline unroll_loops
        Vector get_slot_offset(const Vector &bucket_index, const Vector &block_offset_in_bits, const M mask) const {
            Vector bit_address = block_offset_in_bits + Vector(counters_offset);

            if constexpr (not Vector::avx) {
                if constexpr (buckets_per_block * counter_size <= 32) {
                    const uint32_t counter_mask = (1 << (counter_size * bucket_index.vector)) - 1;
                    const uint32_t counters =
                            *reinterpret_cast<const uint32_t *>(filter + bit_address.vector / 8) & counter_mask;
                    return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters));
                } else if constexpr (buckets_per_block * counter_size <= 64) {
                    const uint64_t counter_mask = (1ull << (counter_size * bucket_index.vector)) - 1;
                    const uint64_t counters =
                            *reinterpret_cast<const uint64_t *>(filter + bit_address.vector / 8) & counter_mask;
                    if (register_size == 32) {
                        return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters)) +
                               simd::sum_bits_in_registers<counter_size, 32 % counter_size>(Vector(counters >> 32));
                    } else {
                        return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters));
                    }
                } else if constexpr (buckets_per_block * counter_size <= 128) {
                    constexpr __uint128_t one = 1;
                    const __uint128_t counter_mask = (one << (counter_size * bucket_index.vector)) - one;
                    const __uint128_t counters =
                            *reinterpret_cast<const __uint128_t *>(filter + bit_address.vector / 8) & counter_mask;
                    if (register_size == 32) {
                        return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters)) +
                               simd::sum_bits_in_registers<counter_size, 32 % counter_size>(Vector(counters >> 32)) +
                               simd::sum_bits_in_registers<counter_size, 64 % counter_size>(Vector(counters >> 64)) +
                               simd::sum_bits_in_registers<counter_size, 96 % counter_size>(Vector(counters >> 96));
                    } else {
                        return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters)) +
                               simd::sum_bits_in_registers<counter_size, 64 % counter_size>(Vector(counters >> 64));
                    }
                } else {
                    Vector count{0};
                    size_t i = 0;
                    if constexpr (counter_to_load < buckets_per_block) {
                        for (; i + counter_to_load <= bucket_index.vector; i += counter_to_load) {
                            const Vector counters = simd::_gatheru_bits<counter_size * counter_to_load, true>(
                                    bit_address, filter, mask);
                            count = count + simd::sum_bits_in_registers<counter_size, 0>(counters);
                            bit_address = bit_address + Vector(counter_size * counter_to_load);
                        }
                    }

                    const Vector counters = simd::_gatheru_bits<counter_size * counter_to_load, true>(bit_address,
                            filter, mask);
                    const Vector counter_mask = Vector((1ull << (counter_size * (bucket_index.vector - i))) - 1);
                    return count + simd::sum_bits_in_registers<counter_size, 0>(counters & counter_mask);
                }
            } else {
                Vector count{0}, index{bucket_index};
                for (size_t i = 0; i <= buckets_per_block; i += counter_to_load) {
                    const Vector counter_mask = (Vector(1) << (index * Vector(counter_size))) - Vector(1);
                    const Vector counters =
                            simd::_gatheru_bits<counter_size * counter_to_load, true>(bit_address, filter, mask) &
                            counter_mask;
                    count = count + simd::sum_bits_in_registers<counter_size, 0>(counters);
                    bit_address = bit_address + Vector(counter_size * counter_to_load);
                    index = Vector(0).add_mask(index - Vector(counter_to_load), index > Vector(counter_to_load));
                }

                return count;
            }
        }

        forceinline unroll_loops
        Vector get_occupied_slots(const Vector &block_offset_in_bits, const M mask) const {
            Vector bit_address = block_offset_in_bits + Vector(counters_offset);

            if constexpr (not Vector::avx and buckets_per_block * counter_size <= 32) {
                const uint32_t counter_mask = (buckets_per_block * counter_size == 32)
                                              ? -1
                                              : (1 << (counter_size * buckets_per_block)) - 1;
                const uint32_t counters =
                        *reinterpret_cast<const uint32_t *>(filter + bit_address.vector / 8) & counter_mask;
                return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters));
            } else if constexpr (buckets_per_block * counter_size <= 64) {
                const uint64_t counter_mask = (buckets_per_block * counter_size == 64)
                                              ? -1
                                              : (1ull << (counter_size * buckets_per_block)) - 1;
                const uint64_t counters =
                        *reinterpret_cast<const uint64_t *>(filter + bit_address.vector / 8) & counter_mask;
                if (register_size == 32) {
                    return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters)) +
                           simd::sum_bits_in_registers<counter_size, 32 % counter_size>(Vector(counters >> 32));
                } else {
                    return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters));
                }
            } else if constexpr (buckets_per_block * counter_size <= 128) {
                constexpr __uint128_t one = 1, minus_one = -1;
                const __uint128_t counter_mask = (buckets_per_block * counter_size == 128)
                                                 ? minus_one
                                                 : (one << (counter_size * buckets_per_block)) - one;
                const __uint128_t counters =
                        *reinterpret_cast<const __uint128_t *>(filter + bit_address.vector / 8) & counter_mask;
                if (register_size == 32) {
                    return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters)) +
                           simd::sum_bits_in_registers<counter_size, 32 % counter_size>(Vector(counters >> 32)) +
                           simd::sum_bits_in_registers<counter_size, 64 % counter_size>(Vector(counters >> 64)) +
                           simd::sum_bits_in_registers<counter_size, 96 % counter_size>(Vector(counters >> 96));
                } else {
                    return simd::sum_bits_in_registers<counter_size, 0>(Vector(counters)) +
                           simd::sum_bits_in_registers<counter_size, 64 % counter_size>(Vector(counters >> 64));
                }
            } else {
                Vector count{0};
                for (size_t i = 0; i + counter_to_load <= buckets_per_block; i += counter_to_load) {
                    const Vector counters = simd::_gatheru_bits<counter_size * counter_to_load, true>(bit_address,
                            filter, mask);
                    count = count + simd::sum_bits_in_registers<counter_size, 0>(counters);
                    bit_address = bit_address + Vector(counter_size * counter_to_load);
                }

                constexpr size_t remaining_counter =
                        buckets_per_block - (buckets_per_block / counter_to_load) * counter_to_load;
                const Vector counters = simd::_gatheru_bits<counter_size * counter_to_load, true>(bit_address, filter,
                        mask);
                const Vector counter_mask = Vector((1ull << (counter_size * remaining_counter)) - 1);
                return count + simd::sum_bits_in_registers<counter_size, 0>(counters & counter_mask);
            }
        }

        forceinline unroll_loops
        M bucket_contains(const Vector &fingerprint, const Vector &bucket_index, const Vector &block_offset_in_bits,
                          const M mask) const {
            const Vector slot_offset = get_slot_offset(bucket_index, block_offset_in_bits, mask);
            Vector slot_count = get_slot_count(bucket_index, block_offset_in_bits, mask);
            Vector bit_address = block_offset_in_bits + Vector(512) - (slot_count + slot_offset) * Vector(k);

            if constexpr (not Vector::avx) {
                bool found = false;
                size_t i = 0;
                if constexpr (slots_to_load < slots_per_bucket) {
                    for (; i + slots_to_load < slot_count.vector; i += slots_to_load) {
                        const PartialBucket bucket = simd::_gatheru_bits<slots_to_load * k, false>(bit_address, filter,
                                mask).vector;
                        found |= (has_value<k, slots_per_bucket>(bucket, static_cast<Fingerprint>(fingerprint.vector)) &
                                  bucket_mask);
                        bit_address = bit_address + Vector(slots_to_load * k);
                    }
                }

                const PartialBucket bucket = simd::_gatheru_bits<slots_to_load * k, false>(bit_address, filter,
                        mask).vector;
                PartialBucket partialBucket_mask;
                const size_t shift_factor = slots_to_load + i - slot_count.vector;
                if (slots_to_load * k == register_size) {
                    partialBucket_mask = (bucket_mask >> shift_factor) >> (shift_factor * (k - 1));
                } else {
                    partialBucket_mask = bucket_mask >> (shift_factor * k);
                }
                found |= (has_value<k, slots_per_bucket>(bucket, static_cast<Fingerprint>(fingerprint.vector)) &
                          partialBucket_mask);
                return found;
            } else {
                M found = Vector::mask(0);
                for (size_t i = 0; i < slots_per_bucket;) {
                    const Vector bucket = simd::_gatheru_bits<slots_to_load * k, false>(bit_address, filter, mask);

                    if constexpr (k == 8 or k == 16) {
                        uint64_t equal;
                        if constexpr (k == 8) {
                            equal = bucket.template equal<k>(fingerprint.permute_regxbyte());
                        } else {
                            equal = bucket.template equal<k>(fingerprint.permute_regxword());
                        }
                        equal &= Vector(0).template equal<k>(Vector(-1) << (slot_count * Vector(k)));
                        for (size_t j = 1; j < slots_to_load; j <<= 1) {
                            equal |= (equal >> j);
                        }
                        found |= simd::pext64(equal, pext_mask);
                        slot_count = Vector(0).add_mask(slot_count - Vector(slots_to_load),
                                slot_count > Vector(slots_to_load));
                        i += slots_to_load;
                    } else {
                        for (size_t j = 0; j < slots_to_load; j++, i++) {
                            found |= (Vector(i) < slot_count) &
                                     (((bucket >> (j * k)) & Vector(fingerprint_mask)) == fingerprint);
                        }
                    }

                    bit_address = bit_address + Vector(slots_to_load * k);
                }
                return found;
            }
        }

        forceinline
        Vector read_fingerprint(const Vector &slot_index, const Vector &block_offset_in_bits) const {
            if constexpr (optimal_fingerprint_size) {
                const uint8_t *byte_address = filter + (block_offset_in_bits.vector + 512) / 8 -
                                              (slot_index.vector + 1) * sizeof(Fingerprint);
                return Vector(*reinterpret_cast<const Fingerprint *>(byte_address));
            } else {
                const Vector bit_address = block_offset_in_bits + Vector(512 - slot_index.vector * k - k);
                return simd::_gatheru_bits<k, false>(bit_address, filter, true);
            }
        }

        forceinline
        void set_fingerprint(const Vector &slot_index, const Vector &block_offset_in_bits, const Vector &fingerprint) {
            if constexpr (optimal_fingerprint_size) {
                uint8_t *byte_address = filter + (block_offset_in_bits.vector + 512) / 8 -
                                        (slot_index.vector + 1) * sizeof(Fingerprint);
                std::memcpy(byte_address, &fingerprint.vector, sizeof(Fingerprint));
            } else {
                const Vector bit_address = block_offset_in_bits + Vector(512 - slot_index.vector * k - k);
                simd::_scatteru_bits<k, false>(bit_address, filter, fingerprint, true);
            }
        }

        forceinline
        bool is_free(const Vector &address, Vector &bucket_index, Vector &block_offset_in_bits, Vector &slot_count,
                     Vector &occupied_slots, const Vector &offset) const {
            Vector block_address;
            split_address<buckets_per_block>(address, block_address, bucket_index);
            block_offset_in_bits = (offset << 3) + block_address * Vector(block_size);
            slot_count = get_slot_count(bucket_index, block_offset_in_bits, true);
            occupied_slots = get_occupied_slots(block_offset_in_bits, true);

            return occupied_slots.vector != slots_per_block && slot_count.vector != slots_per_bucket;
        }

        forceinline unroll_loops
        void shift_slots_left_and_insert(const Vector &fingerprint, const Vector &start, const Vector &n_fingerprints,
                                         const Vector &block_offset_in_bits) {
            if constexpr (optimal_fingerprint_size) {
                uint8_t *block = filter + (block_offset_in_bits.vector + 512) / 8;
                uint8_t *begin = block - n_fingerprints.vector * sizeof(Fingerprint);
                uint8_t *end = block - start.vector * sizeof(Fingerprint);

                if (end - begin == sizeof(Fingerprint)) {
                    std::memcpy(begin - sizeof(Fingerprint), begin, sizeof(Fingerprint));
                } else if (end - begin > sizeof(Fingerprint)) {
                    std::memmove(begin - sizeof(Fingerprint), begin, end - begin);
                }
                std::memcpy(end - sizeof(Fingerprint), &fingerprint.vector, sizeof(Fingerprint));
            } else {
                size_t i = start.vector;
                Vector bit_address = block_offset_in_bits + Vector(512) - n_fingerprints * Vector(k);
                for (; i + slots_to_load <= n_fingerprints.vector; i += slots_to_load) {
                    Vector value = simd::_gatheru_bits<slots_to_load * k, false>(bit_address, filter, true);
                    simd::_scatteru_bits<slots_to_load * k, false>(bit_address - Vector(k), filter, value, true);
                    bit_address = bit_address + Vector(slots_to_load * k);
                }

                for (; i + 1 <= n_fingerprints.vector; i++) {
                    Vector value = simd::_gatheru_bits<k, false>(bit_address, filter, true);
                    simd::_scatteru_bits<k, false>(bit_address - Vector(k), filter, value, true);
                    bit_address = bit_address + Vector(k);
                }

                simd::_scatteru_bits<k, false>(bit_address - Vector(k), filter, fingerprint, true);
            }
        }

        forceinline unroll_loops
        void shift_slots_right_and_insert(const Vector &fingerprint, const Vector &start, const Vector &n_fingerprints,
                                          const Vector &block_offset_in_bits) {
            if (optimal_fingerprint_size) {
                uint8_t *block = filter + (block_offset_in_bits.vector + 512) / 8;
                uint8_t *begin = block - n_fingerprints.vector * sizeof(Fingerprint);
                uint8_t *end = block - start.vector * sizeof(Fingerprint);
                if (end - begin - sizeof(Fingerprint) != 0) {
                    std::memmove(begin + sizeof(Fingerprint), begin, end - begin - sizeof(Fingerprint));
                }
                std::memcpy(begin, &fingerprint.vector, sizeof(Fingerprint));
            } else {
                size_t j = start.vector + 1;
                Vector bit_address = block_offset_in_bits + Vector(512) - start * Vector(k);
                for (; j + slots_to_load <= n_fingerprints.vector; j += slots_to_load) {
                    bit_address = bit_address - Vector(slots_to_load * k);
                    Vector value = simd::_gatheru_bits<slots_to_load * k, false>(bit_address - Vector(k), filter, true);
                    simd::_scatteru_bits<slots_to_load * k, false>(bit_address, filter, value, true);
                }

                for (; j + 1 <= n_fingerprints.vector; j++) {
                    bit_address = bit_address - Vector(k);
                    Vector value = simd::_gatheru_bits<k, false>(bit_address - Vector(k), filter, true);
                    simd::_scatteru_bits<k, false>(bit_address, filter, value, true);
                }

                simd::_scatteru_bits<k, false>(bit_address - Vector(k), filter, fingerprint, true);
            }
        }

        forceinline unroll_loops
        void bucket_insert(const Vector &fingerprint, const Vector &bucket_index, const Vector &block_offset_in_bits,
                           const Vector &slot_offset, const Vector &slot_count, const Vector occupied_slots) {
            shift_slots_left_and_insert(fingerprint, slot_offset + slot_count, occupied_slots, block_offset_in_bits);
            set_slot_count(bucket_index, block_offset_in_bits, slot_count + Vector(1), true);
        }

        forceinline
        M contains(const Vector &value, const size_t index, const M mask) const {
            const Vector hash = Hasher::hash(value);
            const Vector fingerprint = morton_fingerprint<k>(addresser, hash);
            const Vector offset = offset_vertical(index);
            const Vector address1 = addresser.compute_address_vertical(index, simd::extractAddressBits(hash));
            Vector block_address1, bucket_index1;
            split_address<buckets_per_block>(address1, block_address1, bucket_index1);
            const Vector block_offset_in_bits1 = (offset << 3) + block_address1 * Vector(block_size);

            if constexpr (not Vector::avx and ota) {
                M overflow = get_ota(bucket_index1, block_offset_in_bits1, mask);
                if (not overflow) {
                    return mask & bucket_contains(fingerprint, bucket_index1, block_offset_in_bits1, mask);
                }
            }

            const Vector address2 = addresser.compute_alternative_address_vertical(index, address1, fingerprint);
            Vector block_address2, bucket_index2;
            split_address<buckets_per_block>(address2, block_address2, bucket_index2);
            const Vector block_offset_in_bits2 = (offset << 3) + block_address2 * Vector(block_size);

            if constexpr (Vector::avx and ota) {
                M overflow = get_ota(bucket_index1, block_offset_in_bits1, mask);
                return mask & (bucket_contains(fingerprint, bucket_index1, block_offset_in_bits1, mask) |
                               bucket_contains(fingerprint, bucket_index2, block_offset_in_bits2, overflow & mask));
            } else {
                return mask & (bucket_contains(fingerprint, bucket_index1, block_offset_in_bits1, mask) |
                               bucket_contains(fingerprint, bucket_index2, block_offset_in_bits2, mask));
            }
        }

        forceinline
        bool evict_victim(const size_t index, Vector &address, Vector &bucket_index, Vector &block_offset_in_bits,
                          Vector &slot_count, Vector &occupied_slots, const Vector slot_offset, Vector &fingerprint,
                          const Vector offset) {
            Vector victim_fingerprint;

            if (slot_count.vector == slots_per_bucket) {
                // bucket overflow
                size_t random_index;
                if constexpr (math::is_power_of_two(slots_per_bucket)) {
                    random_index = rng.rand<slots_per_bucket_log>();
                } else {
                    random_index = rng.rand<32>() % slots_per_bucket;
                }

                if constexpr (optimal_fingerprint_size) {
                    Fingerprint *victim =
                            reinterpret_cast<Fingerprint *>(filter + (block_offset_in_bits.vector + 512) / 8) -
                            (slot_offset.vector + random_index + 1);
                    victim_fingerprint.vector = *victim;
                    std::memcpy(victim, &fingerprint.vector, sizeof(Fingerprint));
                } else {
                    const Vector bit_address =
                            block_offset_in_bits + Vector(512 - (slot_offset.vector + random_index + 1) * k);
                    victim_fingerprint = simd::_gatheru_bits<k, false>(bit_address, filter, true);
                    simd::_scatteru_bits<k, false>(bit_address, filter, fingerprint, true);
                }
                set_ota(bucket_index, block_offset_in_bits, true);
                address = addresser.compute_alternative_address_horizontal(index, address, victim_fingerprint);
                fingerprint = victim_fingerprint;
                return is_free(address, bucket_index, block_offset_in_bits, slot_count, occupied_slots, offset);
            } else {

                // block overflow
                size_t i, j;
                Vector v_i, v_address;
                Vector victim_address, victim_bucket_index, victim_block_offset_in_bits, victim_slot_count, victim_occupied_slots;

                for (i = bucket_index.vector + 1, j = (slot_offset + slot_count).vector, v_address =
                        address - bucket_index + Vector(i);
                     i < buckets_per_block; i++, v_address = v_address + Vector(1)) {

                    Vector t_slot_count = get_slot_count(Vector(i), block_offset_in_bits, true);
                    for (size_t l = 0; l < t_slot_count.vector; l++, j++) {
                        v_i = Vector(i);
                        victim_fingerprint = read_fingerprint(Vector(j), block_offset_in_bits);
                        victim_address = addresser.compute_alternative_address_horizontal(index, v_address,
                                victim_fingerprint);

                        if (is_free(victim_address, victim_bucket_index, victim_block_offset_in_bits, victim_slot_count,
                                victim_occupied_slots, offset)) {

                            shift_slots_left_and_insert(fingerprint, slot_offset + slot_count, Vector(j),
                                    block_offset_in_bits);
                            set_slot_count(bucket_index, block_offset_in_bits, slot_count + Vector(1), true);
                            set_slot_count(Vector(i), block_offset_in_bits, t_slot_count - Vector(1), true);
                            set_ota(Vector(i), block_offset_in_bits, true);

                            address = victim_address;
                            fingerprint = victim_fingerprint;
                            bucket_index = victim_bucket_index;
                            block_offset_in_bits = victim_block_offset_in_bits;
                            // can't reuse victim_slot_count, might have changed in beetween by increment of bucket_index
                            slot_count = get_slot_count(victim_bucket_index, block_offset_in_bits, true);
                            occupied_slots = victim_occupied_slots;
                            return true;
                        }
                    }
                }

                for (i = 0, j = 0, v_address = address - bucket_index;
                     i < bucket_index.vector; i++, v_address = v_address + Vector(1)) {

                    Vector t_slot_count = get_slot_count(Vector(i), block_offset_in_bits, true);
                    for (size_t l = 0; l < t_slot_count.vector; l++, j++) {
                        v_i = Vector(i);
                        victim_fingerprint = read_fingerprint(Vector(j), block_offset_in_bits);
                        victim_address = addresser.compute_alternative_address_horizontal(index, v_address,
                                victim_fingerprint);

                        if (is_free(victim_address, victim_bucket_index, victim_block_offset_in_bits, victim_slot_count,
                                victim_occupied_slots, offset)) {

                            shift_slots_right_and_insert(fingerprint, Vector(j), slot_offset, block_offset_in_bits);
                            set_slot_count(bucket_index, block_offset_in_bits, slot_count + Vector(1), true);
                            set_slot_count(Vector(i), block_offset_in_bits, t_slot_count - Vector(1), true);
                            set_ota(Vector(i), block_offset_in_bits, true);

                            address = victim_address;
                            fingerprint = victim_fingerprint;
                            bucket_index = victim_bucket_index;
                            block_offset_in_bits = victim_block_offset_in_bits;
                            slot_count = get_slot_count(victim_bucket_index, block_offset_in_bits, true);
                            occupied_slots = victim_occupied_slots;
                            return true;
                        }
                    }
                }

                if (v_i > bucket_index) {
                    shift_slots_left_and_insert(fingerprint, slot_offset + slot_count, occupied_slots - Vector(1),
                            block_offset_in_bits);
                } else {
                    set_fingerprint(slot_offset - Vector(1), block_offset_in_bits, fingerprint);
                }

                set_slot_count(bucket_index, block_offset_in_bits, slot_count + Vector(1), true);
                set_slot_count(v_i, block_offset_in_bits,
                        get_slot_count(Vector(v_i), block_offset_in_bits, true) - Vector(1), true);
                set_ota(v_i, block_offset_in_bits, true);

                address = victim_address;
                fingerprint = victim_fingerprint;
                bucket_index = victim_bucket_index;
                block_offset_in_bits = victim_block_offset_in_bits;
                slot_count = get_slot_count(victim_bucket_index, block_offset_in_bits, true);
                occupied_slots = victim_occupied_slots;
                return false;
            }
        }

        forceinline
        M insert(const Vector &hash, const Vector &offset, const size_t index) {
            if constexpr (Vector::avx) {
                return Vector::mask(0);
            } else {
                Vector fingerprint = morton_fingerprint<k>(addresser, hash);
                Vector address = addresser.compute_address_horizontal(index, simd::extractAddressBits(hash));
                Vector bucket_index, block_offset_in_bits, slot_count, occupied_slots;

                if (is_free(address, bucket_index, block_offset_in_bits, slot_count, occupied_slots, offset)) {
                    const Vector slot_offset = get_slot_offset(bucket_index, block_offset_in_bits, true);
                    bucket_insert(fingerprint, bucket_index, block_offset_in_bits, slot_offset, slot_count,
                            occupied_slots);
                    return true;
                }

                set_ota(bucket_index, block_offset_in_bits, true);
                address = addresser.compute_alternative_address_horizontal(index, address, fingerprint);
                bool free = is_free(address, bucket_index, block_offset_in_bits, slot_count, occupied_slots, offset);
                for (size_t i = 0; i < MAX_ITERATION; i++) {
                    const Vector slot_offset = get_slot_offset(bucket_index, block_offset_in_bits, true);
                    if (free) {
                        bucket_insert(fingerprint, bucket_index, block_offset_in_bits, slot_offset, slot_count,
                                occupied_slots);
                        return true;
                    }

                    free = evict_victim(index, address, bucket_index, block_offset_in_bits, slot_count, occupied_slots,
                            slot_offset, fingerprint, offset);
                }

                return false;
            }
        }

        forceinline
        M insert(const Vector &value, const size_t index, const M mask) {
            if constexpr (Vector::avx) {
                return Vector::mask(0);
            }
            const Vector offset = offset_horizontal(index);
            const Vector hash = Hasher::hash(value);
            return insert(hash, offset, index);
        }

        forceinline
        M construct(Vector begin, const Vector &end, const T *values, size_t index) {
            if constexpr (Vector::avx) {
                return Vector::mask(0);
            }
            rng.reset();
            M success = Vector::mask(1);
            const Vector offset = offset_horizontal(index);
            for (M mask; (mask = (begin < end) & success); begin = begin + Vector(1)) {
                const Vector v = begin.gather(values, mask);
                const Vector h = Hasher::hash(v);
                success &= insert(h, offset, index);
            }

            return success;
        }

        forceinline
        size_t size(size_t index) const {
            return addresser.get_size(index);
        }

        forceinline
        size_t length() const {
            return n_bytes;
        }

        forceinline
        size_t retries() const {
            return 0;
        }

        forceinline
        T *data() const {
            return filter;
        }
    };

} // filters::cuckoo
