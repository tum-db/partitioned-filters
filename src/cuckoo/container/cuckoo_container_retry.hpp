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

    template<Variant variant, size_t associativity, size_t buckets_per_block, size_t ota_size, size_t k, parameter::Partitioning partitioning, parameter::RegisterSize registerSize, parameter::SIMD simd, parameter::AddressingMode addressingMode, parameter::HashingMode hashingMode>
    struct CuckooContainer<true, variant, associativity, buckets_per_block, ota_size, k, partitioning, registerSize,
            simd, addressingMode, hashingMode> {

        using Vector = simd::Vector<registerSize, simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        using Container = CuckooContainer<false, variant, associativity, buckets_per_block, ota_size, k, partitioning,
                registerSize, simd, addressingMode, hashingMode>;

        static constexpr bool partitioned = partitioning == parameter::Partitioning::Enabled;

        T *seeds;
        Container container;
        size_t s, n_partitions, n_retries{0};

        // types used for fallback (always scalar)
        using FallbackVector = simd::Vector<registerSize, parameter::SIMD::Scalar>;
        using FallbackContainer = CuckooContainer<false, variant, associativity, buckets_per_block, ota_size, k,
                parameter::Partitioning::Disabled, registerSize, parameter::SIMD::Scalar, addressingMode, hashingMode>;

        CuckooContainer() : seeds(nullptr) {
        }

        explicit CuckooContainer(CuckooContainer &&other) : seeds(other.seeds), container(std::move(other.container)),
                                                            s(other.s), n_partitions(other.n_partitions),
                                                            n_retries(other.n_retries) {
            other.seeds = nullptr;
        }

        CuckooContainer &operator=(CuckooContainer &&other) noexcept {
            if (this != &other) {
                if (seeds) {
                    free(seeds);
                }

                container = std::move(other.container);
                seeds = other.seeds;
                s = other.s;
                n_partitions = other.n_partitions;
                n_retries = other.n_retries;

                other.seeds = nullptr;
            }
            return *this;
        }

        CuckooContainer(size_t s, size_t n_partitions, const T *histogram) : container(s, n_partitions, histogram),
                                                                             s(s), n_partitions(n_partitions) {
            seeds = simd::valloc<Vector>(simd::valign<Vector>(n_partitions * sizeof(T)), 0);
        }

        ~CuckooContainer() {
            if (seeds) {
                free(seeds);
            }
        }

        forceinline
        M contains(const Vector &value, const size_t index, const M mask) const {
            return container.contains(value ^ Vector(seeds[index]), index, mask);
        }

        forceinline
        M insert(const Vector &value, const size_t index, const M mask) {
            return container.insert(value ^ Vector::load(seeds + index), index, mask);
        }

        bool _construct_fallback(const FallbackVector &begin, const FallbackVector &end, const T *values,
                                 const size_t index) {
            bool success = false;
            uint8_t *fallbackFilter = container.filter + ((partitioned)
                                                          ? container.offsets[index]
                                                          : 0);
            size_t n_bytes = (partitioned)
                             ? ((index + 1 < n_partitions)
                                ? container.offsets[index + 1]
                                : container.n_bytes) - container.offsets[index]
                             : container.n_bytes;

            // create fallback container, which will put filter at the intended position in the original container
            FallbackContainer fallbackContainer(fallbackFilter, (end - begin).vector, s);

            for (size_t j = 0; j < hash::n_seeds() and not success; j++) {
                n_retries++;
                std::memset(fallbackFilter, 0, n_bytes);

                rng.reset();
                success = true;
                FallbackVector seed = hash::single_seed<FallbackVector>(j);
                FallbackVector i = begin;
                for (; i < end and success; i = i + FallbackVector(1)) {
                    const FallbackVector v = i.gather(values, true);
                    success &= fallbackContainer.insert(v ^ seed, 0, true);
                }

                seeds[index] = seed.vector;
            }

            return success;
        }

        forceinline
        M construct(const Vector &begin, const Vector &end, const T *values, size_t index) {
            M success = container.construct(begin, end, values, index);
            if (success == Vector::mask(1)) {
                return success;
            } else {
                M failed = ~success & ((partitioned)
                                       ? Vector::mask(1)
                                       : 1);
                while (failed) {
                    size_t set_bit = __builtin_ctz(failed);
                    const FallbackVector fallback_begin = FallbackVector(begin.elements[set_bit]);
                    const FallbackVector fallback_end = FallbackVector(end.elements[set_bit]);
                    if (fallback_begin < fallback_end and not _construct_fallback(fallback_begin, fallback_end, values, index + set_bit)) {
                        return 0;
                    }
                    failed ^= (1ull << set_bit);
                }

                return Vector::mask(1);
            }
        }

        forceinline
        size_t size(size_t index) const {
            return container.size(index);
        }

        forceinline
        size_t length() const {
            return container.length();
        }

        forceinline
        size_t retries() const {
            return n_retries;
        }

        forceinline
        T *data() const {
            return container.data();
        }
    };

} // filters::cuckoo
