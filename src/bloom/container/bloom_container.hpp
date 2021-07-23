#pragma once

#include <cstdint>
#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include <cstring>
#include <partition/partition_set.hpp>
#include <simd/helper.hpp>

namespace filters::bloom {

    template<BlockSize blockSize, parameter::Partitioning partitioning, typename Vector, typename Addresser>
    struct BloomContainer {

        using T = typename Vector::T;

        static constexpr size_t address_bits = Addresser::address_bits;

        Addresser addresser;
        T *filter;
        size_t n_bytes;
        T *offsets;

        BloomContainer() : filter(nullptr), n_bytes(0), offsets(nullptr) {
        }

        explicit BloomContainer(BloomContainer &&other) : addresser(std::move(other.addresser)), filter(other.filter),
                                                          n_bytes(other.n_bytes), offsets(other.offsets) {
            other.filter = nullptr;
            other.offsets = nullptr;
        }

        BloomContainer &operator=(BloomContainer &&other) noexcept {
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

        BloomContainer(size_t k, size_t s, size_t n_partitions, const T *histogram) {
            T *h = new T[n_partitions];

            for (size_t i = 0; i < n_partitions; i++) {
                h[i] = histogram[i] * k * s / 100 / static_cast<size_t>(blockSize) + 1;
            }

            addresser = std::move(Addresser(h, n_partitions));
            delete[](h);

            T last_offset = 0;

            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                offsets = simd::valloc<Vector>(simd::valign<Vector>(n_partitions * sizeof(T)), 0);
                for (size_t i = 0; i < n_partitions; i++) {
                    size_t n_blocks = addresser.get_size(i);

                    offsets[i] = last_offset;
                    last_offset += n_blocks;
                }
            } else {
                offsets = nullptr;
                last_offset = addresser.get_size(0);
            }

            n_bytes = simd::valign<Vector>(last_offset * static_cast<size_t>(blockSize) / 8);
            filter = simd::valloc<Vector>(n_bytes, 0, true);
        }

        ~BloomContainer() {
            if (offsets) {
                free(offsets);
            }
            if (filter) {
                simd::vfree(filter, n_bytes, true);
            }
        }

        forceinline
        Vector compute_offset_vertical(const size_t index, const Vector &address) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                const Vector offset = Vector(offsets[index]);
                return offset + addresser.compute_address_vertical(index, address);
            } else {
                return addresser.compute_address_vertical(index, address);
            }
        }

        forceinline
        Vector compute_offset_horizontal(const size_t index, const Vector &address) const {
            if constexpr (partitioning == parameter::Partitioning::Enabled) {
                const Vector offset = Vector::load(offsets + index);
                return offset + addresser.compute_address_horizontal(index, address);
            } else {
                return addresser.compute_address_horizontal(index, address);
            }
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
        T *data() const {
            return filter;
        }
    };
} // filters::bloom
