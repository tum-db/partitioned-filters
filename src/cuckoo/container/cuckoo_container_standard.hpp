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
#include "cuckoo_container_base.hpp"
#include "cuckoo_helper.hpp"

namespace filters::cuckoo {

    template<size_t associativity, size_t k, parameter::Partitioning partitioning, parameter::RegisterSize registerSize, parameter::SIMD simd, parameter::AddressingMode addressingMode, parameter::HashingMode hashingMode>
    struct CuckooContainer<false, Variant::Standard, associativity, 0, 0, k, partitioning, registerSize, simd,
            addressingMode, hashingMode> {

        using Vector = simd::Vector<registerSize, simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;
        using Addresser = addresser::Addresser<addressingMode, Vector>;
        using Hasher = hash::Hasher<hashingMode, Vector, 0>;

        static_assert(k * associativity <= static_cast<size_t>(Vector::registerSize),
                "Bucket is larger than Register Size!");

        static constexpr size_t bits_per_bucket = k * associativity;

        using Fingerprint = std::conditional_t<k <= 8, uint8_t,
                std::conditional_t<k <= 16, uint16_t, std::conditional_t<k <= 32, uint32_t, uint64_t>>>;
        using Bucket = std::conditional_t<bits_per_bucket <= 8, uint8_t,
                std::conditional_t<bits_per_bucket <= 16, uint16_t,
                        std::conditional_t<bits_per_bucket <= 32, uint32_t, uint64_t>>>;

        static constexpr bool optimal_fingerprint_size = (k == 8) or (k == 16) or (k == 32);
        static constexpr Bucket fingerprint_mask = (1ull << k) - 1;
        static constexpr size_t slots_per_bucket = associativity;
        static constexpr size_t slots_per_bucket_log = math::const_log2(slots_per_bucket);
        static constexpr size_t register_size = static_cast<size_t>(Vector::registerSize);

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
                h[i] = math::align_number(histogram[i] * s / 100, associativity) / associativity;
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
                            math::align_number(n_buckets * bits_per_bucket, static_cast<size_t>(Vector::registerSize)) /
                            8;

                    // unaligned loads access two registers
                    // we need one extra register between next partition to avoid race-conditions
                    if (not optimal_fingerprint_size) {
                        last_offset += static_cast<size_t>(Vector::registerSize) / 8;
                    }
                }
            } else {
                offsets = nullptr;
                last_offset = math::align_number(this->addresser.get_size(0) * bits_per_bucket,
                        static_cast<size_t>(Vector::registerSize)) / 8;
            }

            n_bytes = simd::valign<Vector>(last_offset);
            if (n_bytes > 0) {
                filter = reinterpret_cast<uint8_t *>(simd::valloc<Vector>(n_bytes + 64, 0, true));
            }
        }

        CuckooContainer(uint8_t *filter, const size_t length, const size_t s) : filter(filter), offsets(nullptr) {
            static_assert(partitioning == parameter::Partitioning::Disabled);

            T histogram = math::align_number(length * s / 100, associativity) / associativity;
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

        forceinline unroll_loops
        M bucket_contains(const Vector &fingerprint, const Vector &address, const Vector &byte_offset,
                          const M mask) const {
            if constexpr (not Vector::avx and optimal_fingerprint_size) {
                // has_value is faster than a loop
                auto *bucket = reinterpret_cast<T *>(filter + byte_offset.vector +
                                                     address.vector * bits_per_bucket / 8);
                return has_value<k, slots_per_bucket>(reinterpret_cast<unaligned<T> *>(bucket)->value, fingerprint.vector) != 0;
            } else if constexpr (not Vector::avx) {
                const T bucket = simd::gatheru_bits<bits_per_bucket>(address, filter, byte_offset, mask).vector;
                return has_value<k, slots_per_bucket>(bucket, fingerprint.vector) != 0;
            } else {
                const Vector bucket = simd::gatheru_bits<bits_per_bucket>(address, filter, byte_offset, mask);

                M res = Vector::mask(0);
                // TODO: k == 32
                if constexpr (k == 8 or k == 16) {
                    uint64_t equal;
                    if constexpr (k == 8) {
                        equal = bucket.template equal<k>(fingerprint.permute_regxbyte());
                    } else {
                        equal = bucket.template equal<k>(fingerprint.permute_regxword());
                    }
                    for (size_t i = 1; i < slots_per_bucket; i <<= 1) {
                        equal |= (equal >> i);
                    }
                    res = simd::pext64(equal, pext_mask);
                } else {
                    for (size_t i = 0; i < slots_per_bucket * k; i += k) {
                        res |= (((bucket >> i) & Vector(fingerprint_mask)) == fingerprint);
                    }
                }
                return res;
            }
        }

        forceinline unroll_loops
        M bucket_insert(const Vector &fingerprint, const Vector &address, const Vector &byte_offset, const M mask) {
            if constexpr (not Vector::avx and optimal_fingerprint_size) {
                auto *bucket = reinterpret_cast<Bucket *>(filter + byte_offset.vector +
                                                          address.vector * bits_per_bucket / 8);

                // This is faster for small filters
                // If the number of elements exceeds 10 million, a loop is faster for slots_per_bucket < 4
                const Bucket zero = has_zero<k, slots_per_bucket>(reinterpret_cast<unaligned<T> *>(bucket)->value);
                const size_t zero_index = simd::tzcount(zero);
                if (zero > 0) {
                    reinterpret_cast<Fingerprint *>(bucket)[zero_index / k] = fingerprint.vector;
                    return Vector::mask(1);
                }
                return Vector::mask(0);
            } else if constexpr (not Vector::avx) {
                Bucket bucket = simd::gatheru_bits<bits_per_bucket>(address, filter, byte_offset, mask).vector;
                const Bucket zero = has_zero<k, slots_per_bucket>(bucket);
                const size_t zero_index = simd::tzcount(zero);

                if (zero > 0) {
                    bucket |= (fingerprint.vector << zero_index);
                    simd::scatteru_bits<bits_per_bucket>(address, filter, byte_offset, Vector(bucket), mask);
                    return Vector::mask(1);
                }
                return Vector::mask(0);
            } else {
                Vector bucket = simd::gatheru_bits<bits_per_bucket>(address, filter, byte_offset, mask);

                Vector shift = Vector(0);
                M res = Vector::mask(0);
                for (size_t i = 0; i < slots_per_bucket; i++) {
                    Vector slot = (bucket >> shift) & Vector(fingerprint_mask);
                    M m = ~res & (slot == Vector(0));
                    bucket = bucket.or_mask(fingerprint << shift, m);
                    shift = shift + Vector(k);
                    res |= m;
                }
                simd::scatteru_bits<bits_per_bucket>(address, filter, byte_offset, bucket, mask);
                return res & mask;
            }
        }

        forceinline
        M contains(const Vector &value, const size_t index, const M mask) const {
            const Vector hash = Hasher::hash(value);
            const Vector fingerprint = cuckoo_fingerprint<k>(addresser, hash);
            const Vector offset = offset_vertical(index);
            const Vector address1 = addresser.compute_address_vertical(index, simd::extractAddressBits(hash));
            const Vector address2 = addresser.compute_alternative_address_vertical(index, address1, fingerprint);

            return mask & (bucket_contains(fingerprint, address1, offset, mask) |
                           bucket_contains(fingerprint, address2, offset, mask));
        }

        forceinline
        void evict_victim(const size_t index, Vector &address, Vector &fingerprint, const Vector &byte_offset,
                          const M mask) {
            Vector random_index;
            if constexpr (math::is_power_of_two(slots_per_bucket)) {
                random_index = Vector(rng.rand<slots_per_bucket_log>());
            } else {
                // compiler should optimize the division
                random_index = Vector(rng.rand<32>() % slots_per_bucket);
            }
            Vector slot;

            if constexpr (not Vector::avx and optimal_fingerprint_size) {
                auto *bucket = reinterpret_cast<Bucket *>(filter + byte_offset.vector +
                                                          address.vector * bits_per_bucket / 8);
                slot.vector = reinterpret_cast<Fingerprint *>(bucket)[random_index.vector];
                reinterpret_cast<Fingerprint *>(bucket)[random_index.vector] = fingerprint.vector;
            } else {
                const Vector bit_address =
                        Vector(byte_offset << 3) + address * Vector(bits_per_bucket) + random_index * Vector(k);
                slot = simd::_gatheru_bits<k, false>(bit_address, filter, mask);
                simd::_scatteru_bits<k, false>(bit_address, filter, fingerprint, mask);
            }

            address = addresser.compute_alternative_address_horizontal(index, address, slot);
            fingerprint = slot;
        }

        forceinline
        M insert(const Vector &hash, const Vector &offset, const size_t index, M mask) {
            Vector fingerprint = cuckoo_fingerprint<k>(addresser, hash);
            const Vector address1 = addresser.compute_address_horizontal(index, simd::extractAddressBits(hash));

            M inserted = bucket_insert(fingerprint, address1, offset, mask);
            M todo = mask & ~inserted;

            if (inserted == mask) {
                return inserted;
            }

            Vector address2 = addresser.compute_alternative_address_horizontal(index, address1, fingerprint);
            for (size_t i = 0; i < MAX_ITERATION; i++) {
                inserted |= bucket_insert(fingerprint, address2, offset, todo);
                todo = mask & ~inserted;

                if (inserted == mask) {
                    return inserted;
                }

                evict_victim(index, address2, fingerprint, offset, todo);
            }

            return inserted;
        }

        forceinline
        M insert(const Vector &value, const size_t index, const M mask) {
            const Vector offset = offset_horizontal(index);
            const Vector hash = Hasher::hash(value);
            return insert(hash, offset, index, mask);
        }

        forceinline
        M construct(Vector begin, const Vector &end, const T *values, size_t index) {
            rng.reset();
            M success = Vector::mask(1);
            const Vector offset = offset_horizontal(index);
            for (M mask; (mask = (begin < end) & success); begin = begin + Vector(1)) {
                const Vector v = begin.gather(values, mask);
                const Vector h = Hasher::hash(v);
                success &= ~(mask ^ insert(h, offset, index, mask));
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
