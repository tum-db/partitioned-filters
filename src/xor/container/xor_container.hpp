#pragma once

#include <cstdint>
#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include <array>
#include <partition/partition_set.hpp>
#include "xor_stack.hpp"

namespace filters::nxor {

    template<Variant variant, size_t k, parameter::Partitioning partitioning, typename Hasher, typename Vector,
            typename Addresser>
    struct XorContainer {

        using Stack = XorStack<variant, Hasher, Vector, Addresser>;
        using T = typename Vector::T;
        using M = typename Vector::M;
        using Fingerprint = std::conditional_t<k <= 8, uint8_t, std::conditional_t<k <= 16, uint16_t, uint32_t>>;
        using O = std::conditional_t<partitioning == parameter::Partitioning::Enabled, T *, T>;

        Addresser addresser;
        uint8_t *fingerprints;
        size_t n_bytes;
        std::array<O, 3> offsets{0};
        O seeds;
        Vector base;

        XorContainer() : fingerprints(nullptr), n_bytes(0), seeds(0) {
        };

        XorContainer(XorContainer &&other) noexcept: addresser(std::move(other.addresser)),
                                                     fingerprints(other.fingerprints), n_bytes(other.n_bytes),
                                                     seeds(other.seeds), base(other.base) {
            other.fingerprints = nullptr;
            for (size_t i = 0; i < offsets.size(); i++) {
                this->offsets[i] = other.sets[i];
                other.offsets[i] = 0;
            }
        }

        XorContainer &operator=(XorContainer &&other) noexcept {
            if (this != &other) {
                addresser = std::move(other.addresser);
                n_bytes = other.n_bytes;
                base = other.base;

                if (this->fingerprints) {
                    simd::vfree(this->fingerprints, n_bytes, true);
                }
                this->fingerprints = other.fingerprints;
                other.fingerprints = nullptr;

                for (size_t i = 0; i < offsets.size(); i++) {
                    if constexpr (partitioning == parameter::Partitioning::Enabled) {
                        if (this->offsets[i]) {
                            free(this->offsets[i]);
                        }
                    }

                    this->offsets[i] = other.offsets[i];
                    other.offsets[i] = 0;
                }

                if constexpr (partitioning == parameter::Partitioning::Enabled) {
                    if (this->seeds) {
                        free(this->seeds);
                    }
                }

                this->seeds = other.seeds;
                other.seeds = 0;
            }

            return *this;
        }

        XorContainer(size_t s, size_t n_partitions, const T *histogram) {
            T *h = new T[n_partitions];

            for (size_t i = 0; i < n_partitions; i++) {
                if constexpr (variant == Variant::Standard) {
                    h[i] = math::align_number(histogram[i] * s / 100, 3ul) / 3;
                } else {
                    h[i] = math::align_number(histogram[i] * s / 100, FUSE_SEGMENTS + 2) / (FUSE_SEGMENTS + 2);
                }
                if (h[i] == 0) {
                    h[i] = 1;
                }
            }

            addresser = std::move(Addresser(h, n_partitions));
            delete[](h);

            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                for (auto &offset : offsets) {
                    offset = simd::valloc<Vector>(simd::valign<Vector>(n_partitions * sizeof(T)));
                }
                seeds = simd::valloc<Vector>(simd::valign<Vector>(n_partitions * sizeof(T)), 0);
            } else {
                seeds = 0;
            }

            T last_offset = 0;
            for (size_t i = 0; i < n_partitions; i++) {
                size_t n_fingerprints = this->addresser.get_size(i);

                for (auto &offset : offsets) {
                    if constexpr (partitioning == parameter::Partitioning::Enabled) {
                        offset[i] = last_offset;
                    } else {
                        offset = last_offset;
                    }
                    last_offset += n_fingerprints;
                }

                if constexpr (variant == Variant::Fuse) {
                    last_offset += n_fingerprints * (FUSE_SEGMENTS - 1);
                }

                // unaligned loads access two registers
                // we need one extra register between next partition to avoid race-conditions
                if (k != 8 and k != 16 and k != 32) {
                    last_offset += static_cast<size_t>(Vector::registerSize) / 8;
                }
            }

            n_bytes = simd::valign<Vector>(math::align_number(static_cast<size_t>(last_offset * k), 8ul) / 8);
            fingerprints = reinterpret_cast<uint8_t *>(simd::valloc<Vector>(n_bytes, 0, true));

            for (size_t i = 0; i < Vector::n_elements; i++) {
                base.elements[i] = i;
            }
        }

        XorContainer(uint8_t *fingerprints, const size_t length, const size_t s, const T offset0, const T offset1,
                     const T offset2) : fingerprints(fingerprints), n_bytes(0), seeds(0) {
            static_assert(partitioning == parameter::Partitioning::Disabled);

            T histogram;
            if constexpr (variant == Variant::Standard) {
                histogram = math::align_number(length * s / 100, 3ul) / 3;
            } else {
                histogram = math::align_number(length * s / 100, FUSE_SEGMENTS + 2) / (FUSE_SEGMENTS + 2);
            }
            if (histogram == 0) {
                histogram = 1;
            }
            addresser = std::move(Addresser(&histogram, 1));

            offsets[0] = offset0;
            offsets[1] = offset1;
            offsets[2] = offset2;
        }

        ~XorContainer() {
            if (fingerprints && n_bytes > 0) {
                simd::vfree(fingerprints, n_bytes, true);
            }
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                if (seeds) {
                    free(seeds);
                }
                for (auto *offset : offsets) {
                    if (offset) {
                        free(offset);
                    }
                }
            }
        }

        forceinline
        size_t length() const {
            return n_bytes;
        }

        template<size_t a>
        forceinline
        Vector offset_vertical(const size_t index) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                return Vector(std::get<a>(offsets)[index]);
            } else {
                return Vector(std::get<a>(offsets));
            }
        }

        template<size_t a>
        forceinline
        Vector offset_horizontal(const size_t index) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                return Vector::load(std::get<a>(offsets) + index);
            } else {
                return Vector(std::get<a>(offsets));
            }
        }

        forceinline
        Vector seed_vertical(const size_t index) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                return Vector(seeds[index]);
            } else {
                return Vector(seeds);
            }
        }

        forceinline
        Vector seed_horizontal(const size_t index) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                return Vector::load(seeds + index);
            } else {
                return Vector(seeds);
            }
        }

        forceinline
        void set_seed(const T &seed, const size_t index) {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                seeds[index] = seed;
            } else {
                seeds = seed;
            }
        }

        forceinline
        Vector get_fingerprint(const Vector &address, const M &mask) const {
            if constexpr (not Vector::avx and (k == 8 or k == 16 or k == 32)) {
                return Vector(reinterpret_cast<const Fingerprint *>(fingerprints)[address.vector]);
            } else {
                return simd::gatheru_bits<k>(address, fingerprints, mask);
            }
        }

        forceinline
        void set_fingerprint(const Vector &fingerprint, const Vector &address, const M &mask) {
            // use 8/16-bit store functions for scalar Vectors
            if constexpr (not Vector::avx and (k == 8 or k == 16 or k == 32)) {
                reinterpret_cast<Fingerprint *>(fingerprints)[address.vector] = fingerprint.vector;
            } else {
                simd::scatteru_bits<k>(address, fingerprints, fingerprint, mask);
            }
        }

        template<size_t set, size_t rf>
        forceinline
        Vector compute_address_horizontal(const Vector &hash, const Vector &offset, const size_t index) const {
            return offset + addresser.compute_address_horizontal(index, simd::extractAddressBits(xor_rotate<rf>(hash)));
        }

        template<size_t set, size_t rf>
        forceinline
        Vector compute_address_vertical(const Vector &hash, const Vector &offset, const size_t index) const {
            return offset + addresser.compute_address_vertical(index, simd::extractAddressBits(xor_rotate<rf>(hash)));
        }

        template<size_t set0, size_t rf0, size_t set1, size_t rf1, size_t set2, size_t rf2>
        forceinline
        void assign_standard(const Stack &stack, const Vector &offset0, const Vector &offset1, const Vector &offset2,
                             const size_t index, const size_t run_index, M mask) {

            Vector begin = stack.runs[run_index - 1];
            const Vector end = stack.runs[run_index];

            for (M m; (m = mask & (begin < end)); begin = begin + Vector(1)) {
                const Vector hash = begin.gather(stack.stack, m);
                const Vector fingerprint0 = xor_fingerprint(hash);

                const Vector address0 = compute_address_horizontal<set0, rf0>(hash, offset0, index);
                const Vector address1 = compute_address_horizontal<set1, rf1>(hash, offset1, index);
                const Vector address2 = compute_address_horizontal<set2, rf2>(hash, offset2, index);

                const Vector fingerprint1 = get_fingerprint(address1, m);
                const Vector fingerprint2 = get_fingerprint(address2, m);
                const Vector fingerprint = fingerprint0 ^fingerprint1 ^fingerprint2;

                set_fingerprint(fingerprint, address0, m);
            }
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        void assign_fuse(const Stack &stack, const Vector &offset0, const Vector &offset1, const Vector &offset2,
                         const size_t index, M mask) {
            const Vector segment_size = addresser.get_vsize(index);
            const Vector begin = stack.begin;
            Vector end = stack.end;

            for (M m; (m = mask & (begin < end));) {
                end = end.add_mask(Vector(-2), m);
                const Vector hash = end.gather(stack.stack, m);
                const Vector address = (end + Vector(1)).gather(stack.stack, m);
                const Vector segment = xor_segment<Variant::Fuse>(hash) * segment_size;

                const Vector address0 = segment + compute_address_horizontal<0, rf0>(hash, offset0, index);
                const Vector address1 = segment + compute_address_horizontal<1, rf1>(hash, offset1, index);
                const Vector address2 = segment + compute_address_horizontal<2, rf2>(hash, offset2, index);

                const Vector fingerprint0 = get_fingerprint(address0, m);
                const Vector fingerprint1 = get_fingerprint(address1, m);
                const Vector fingerprint2 = get_fingerprint(address2, m);
                const Vector fingerprint = xor_fingerprint(hash) ^fingerprint0 ^fingerprint1 ^fingerprint2;

                set_fingerprint(fingerprint, address + offset0, m);
            }
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        void fill(const Stack &stack, const size_t index, const M mask) {
            const Vector offset0 = offset_horizontal<0>(index);
            const Vector offset1 = offset_horizontal<1>(index);
            const Vector offset2 = offset_horizontal<2>(index);

            if constexpr (variant == Variant::Standard) {
                for (size_t i = stack.runs.size() - 1; i > 0; i -= 3) {
                    assign_standard<2, rf2, 0, rf0, 1, rf1>(stack, offset2, offset0, offset1, index, i, mask);
                    assign_standard<1, rf1, 2, rf2, 0, rf0>(stack, offset1, offset2, offset0, index, i - 1, mask);
                    assign_standard<0, rf0, 1, rf1, 2, rf2>(stack, offset0, offset1, offset2, index, i - 2, mask);
                }
            } else {
                assign_fuse<rf0, rf1, rf2>(stack, offset0, offset1, offset2, index, mask);
            }
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        M contains(const Vector &hash, const Vector segment_size, const Vector &offset0, const Vector &offset1,
                   const Vector &offset2, const size_t index, const M mask) const {
            const Vector fingerprint = xor_fingerprint(hash);
            const Vector segment = xor_segment<variant>(hash) * segment_size;

            const Vector address0 = segment + compute_address_vertical<0, rf0>(hash, offset0, index);
            const Vector address1 = segment + compute_address_vertical<1, rf1>(hash, offset1, index);
            const Vector address2 = segment + compute_address_vertical<2, rf2>(hash, offset2, index);

            const Vector fingerprint0 = get_fingerprint(address0, mask);
            const Vector fingerprint1 = get_fingerprint(address1, mask);
            const Vector fingerprint2 = get_fingerprint(address2, mask);

            if constexpr (not Vector::avx and (k == 8 || k == 16 || k == 32)) {
                return (static_cast<Fingerprint>((fingerprint0 ^ fingerprint1 ^ fingerprint2).vector) ==
                        static_cast<Fingerprint>(fingerprint.vector));
            } else {
                return mask & ((fingerprint0 ^ fingerprint1 ^ fingerprint2) == (fingerprint & Vector((1ull << k) - 1)));
            }
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        size_t contains(const T &value, const size_t index) const {
            const Vector seed = seed_vertical(index);

            const Vector segment_size = Vector(addresser.get_size(index));
            const Vector offset0 = offset_vertical<0>(index);
            const Vector offset1 = offset_vertical<1>(index);
            const Vector offset2 = offset_vertical<2>(index);

            const Vector values = Vector(value);
            const Vector hash = Hasher::hash(values ^ seed);

            return contains<rf0, rf1, rf2>(hash, segment_size, offset0, offset1, offset2, index, 1);
        }

        template<size_t rf0, size_t rf1, size_t rf2>
        forceinline
        size_t count(const T *begin, const T *end, const size_t index) {
            const Vector seed = seed_vertical(index);

            const Vector segment_size = Vector(addresser.get_size(index));
            const Vector offset0 = offset_vertical<0>(index);
            const Vector offset1 = offset_vertical<1>(index);
            const Vector offset2 = offset_vertical<2>(index);

            size_t counter = 0;

            for (; begin + Vector::n_elements <= end; begin += Vector::n_elements) {
                const M mask = Vector::mask(1);
                const Vector values = Vector::load(begin);
                const Vector hash = Hasher::hash(values ^ seed);

                counter += Vector::popcount_mask(
                        contains<rf0, rf1, rf2>(hash, segment_size, offset0, offset1, offset2, index, mask));
            }

            if constexpr (Vector::avx) {
                if (begin < end) {
                    const M mask = (1 << (end - begin)) - 1;
                    const Vector values = Vector::load(begin);
                    const Vector hash = Hasher::hash(values ^ seed);

                    counter += Vector::popcount_mask(
                            contains<rf0, rf1, rf2>(hash, segment_size, offset0, offset1, offset2, index, mask));
                }
            }

            return counter;
        }
    };

} // filters::nxor
