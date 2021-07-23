#pragma once

#include <iostream>
#include <filter_base.hpp>
#include <xor/container/xor_set.hpp>
#include <xor/container/xor_queue.hpp>
#include <xor/container/xor_stack.hpp>
#include <xor/container/xor_container.hpp>

namespace filters {

    template<typename FilterParameter, size_t _k, typename OptimizationParameter>
    struct Filter<FilterType::Xor, FilterParameter, _k, OptimizationParameter> {

        using FP = FilterParameter;
        static constexpr size_t k = _k;
        static constexpr nxor::Variant variant = FP::variant;
        static constexpr size_t max_n_retries = FP::max_n_retries;
        using OP = OptimizationParameter;
        static constexpr bool supports_add = false;
        static constexpr bool supports_add_partition = OP::simd == parameter::SIMD::Scalar;

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;
        using Hasher = hash::Hasher<OP::hashingMode, Vector, 3>;
        using Addresser = addresser::Addresser<OP::addressingMode, Vector>;
        using PartitionSet = typename partition::PartitionSet<OP::partitioning, Vector>;

        using Set = nxor::XorSet<variant, Hasher, Vector, Addresser>;
        using Queue = nxor::XorQueue<variant, Hasher, Vector, Addresser>;
        using Stack = nxor::XorStack<variant, Hasher, Vector, Addresser>;
        using Container = nxor::XorContainer<variant, k, OP::partitioning, Hasher, Vector, Addresser>;

        // types used for fallback (always scalar)
        using FallbackVector = simd::Vector<OP::registerSize, parameter::SIMD::Scalar>;
        using FallbackHasher = hash::Hasher<OP::hashingMode, FallbackVector, 3>;
        using FallbackAddresser = addresser::Addresser<OP::addressingMode, FallbackVector>;
        using FallbackSet = nxor::XorSet<variant, FallbackHasher, FallbackVector, FallbackAddresser>;
        using FallbackQueue = nxor::XorQueue<variant, FallbackHasher, FallbackVector, FallbackAddresser>;
        using FallbackStack = nxor::XorStack<variant, FallbackHasher, FallbackVector, FallbackAddresser>;
        using FallbackContainer = nxor::XorContainer<variant, k, parameter::Partitioning::Disabled, FallbackHasher,
                FallbackVector, FallbackAddresser>;

        size_t s;
        size_t n_partitions;
        size_t n_retries;
        Container container;
        task::TaskQueue<OP::multiThreading> queue;

        Filter(size_t s, size_t n_partitions, size_t n_threads, size_t n_tasks_per_level) : s(s),
                                                                                            n_partitions(n_partitions),
                                                                                            n_retries(0),
                                                                                            queue(n_threads,
                                                                                                    n_tasks_per_level) {
        }

        forceinline
        void init(const T *histogram) {
            static_assert(OP::simd == parameter::SIMD::Scalar,
                    "add_partition function only supported for scalar filters!");

            n_partitions = std::max(n_partitions, 1ul);
            container = std::move(Container(s, n_partitions, histogram));
        }

        forceinline
        bool contains(const T &value) const {
            return contains(value, value & (n_partitions - 1));
        }

        forceinline
        bool contains(const T &value, size_t index) const {
            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                return container.template contains<nxor::RF0, nxor::RF1, nxor::RF2>(value, 0);
            } else {
                return container.template contains<nxor::RF0, nxor::RF1, nxor::RF2>(value, index);
            }
        }

        forceinline
        bool add_all(const T *values, size_t length) {
            return add_partition(values, length, 0);
        }

        forceinline
        bool add_partition(const T *values, size_t length, size_t index) {
            static_assert(OP::simd == parameter::SIMD::Scalar,
                    "add_partition function only supported for scalar filters!");

            if constexpr (OP::partitioning == parameter::Partitioning::Disabled) {
                _construct(Vector(0), Vector(length), values, 0);
            } else {
                _construct(Vector(0), Vector(length), values, index);
            }
            return true;
        }

        bool _construct_fallback(const FallbackVector &offset, const FallbackVector &histogram, const T *values,
                                 const size_t index) {
            M mask = FallbackVector::mask(1);

            // create fallback container, which will put filter at the intended position in the original container
            FallbackContainer fallbackContainer(container.fingerprints, histogram.vector, s,
                    container.template offset_vertical<0>(index).elements[0],
                    container.template offset_vertical<1>(index).elements[0],
                    container.template offset_vertical<2>(index).elements[0]);

            for (size_t j = 0; j < max_n_retries && mask; j++) {
                n_retries++;

                // use new seed for this run
                container.set_seed(hash::single_seed<FallbackVector>(j).vector, index);

                // use scalar Set, Queue and Stack
                FallbackSet set(fallbackContainer.addresser, hash::single_seed<FallbackVector>(j), 0);
                FallbackQueue queue(fallbackContainer.addresser, 0);
                FallbackStack stack(histogram);

                // fill them
                set.template fill<nxor::RF0, nxor::RF1, nxor::RF2>(offset, histogram, values);
                queue.fill(set);
                stack.template fill<nxor::RF0, nxor::RF1, nxor::RF2>(set, queue);

                // check if size equals expected size
                mask = (histogram != stack.size());

                // fill container if success
                if (mask == FallbackVector::mask(0)) {
                    fallbackContainer.template fill<nxor::RF0, nxor::RF1, nxor::RF2>(stack, 0, Vector::mask(1));
                }
            }

            return (mask == 0);
        }

        bool _construct(const Vector &offset, const Vector &histogram, const T *values, const size_t index) {
            Set set(container.addresser, Vector(0), index);
            Queue queue(container.addresser, index);
            Stack stack(histogram);

            set.template fill<nxor::RF0, nxor::RF1, nxor::RF2>(offset, histogram, values);
            queue.fill(set);
            stack.template fill<nxor::RF0, nxor::RF1, nxor::RF2>(set, queue);

            M mask = (histogram != stack.size());

            if (mask != Vector::mask(1)) {
                container.template fill<nxor::RF0, nxor::RF1, nxor::RF2>(stack, index, (mask ^ Vector::mask(1)));
            }

            bool success = true;
            while (mask) {
                size_t set_bit = __builtin_ctz(mask);
                const FallbackVector fallback_offset = FallbackVector(offset.elements[set_bit]);
                const FallbackVector fallback_histogram = FallbackVector(histogram.elements[set_bit]);
                success &= _construct_fallback(fallback_offset, fallback_histogram, values, index + set_bit);
                mask ^= (1ull << set_bit);
            }

            return success;
        }

        bool construct(T *values, size_t length) {
            PartitionSet partitions(n_partitions);
            n_partitions = partitions.n_partitions;

            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                bool success = true;
                partitions.init(values, length);
                container = std::move(Container(s, n_partitions, partitions.histogram));

                for (size_t i = 0; i < partitions.n_partitions; i += Vector::n_elements) {
                    const Vector offset = Vector::load(partitions.offsets + i);
                    const Vector histogram = Vector::load(partitions.histogram + i);
                    success &= _construct(offset, histogram, partitions.values, i);
                }

                return success;
            } else {
                partitions.init(values, length, queue);
                std::atomic<uint8_t> success{1};

                queue.add_task([&](size_t) {
                    container = std::move(Container(s, n_partitions, partitions.histogram));
                });
                queue.add_barrier();
                for (size_t i = 0; i < partitions.n_partitions; i += Vector::n_elements) {
                    queue.add_task([this, &partitions, &success, i](size_t) {
                        const Vector offset = Vector::load(partitions.offsets + i);
                        const Vector histogram = Vector::load(partitions.histogram + i);
                        success &= _construct(offset, histogram, partitions.values, i);
                    });
                }
                queue.execute_tasks();
                return success;
            }
        }

        size_t count(T *values, size_t length) {
            PartitionSet partitions(n_partitions);
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {

                partitions.init(values, length);
                size_t counter = 0;

                for (size_t i = 0; i < partitions.n_partitions; i++) {
                    const T *begin = partitions.values + partitions.offsets[i];
                    const T *end = begin + partitions.histogram[i];
                    counter += container.template count<nxor::RF0, nxor::RF1, nxor::RF2>(begin, end, i);
                }

                return counter;
            } else {
                partitions.init(values, length, queue);

                std::atomic<size_t> counter{0};
                size_t remaining_tasks = queue.get_n_tasks_per_level();
                for (size_t i = 0; i < partitions.n_partitions; i++) {
                    // add at least one task
                    size_t n_tasks = 1;
                    if (remaining_tasks > 0) {
                        n_tasks = std::max(remaining_tasks / (partitions.n_partitions - i), static_cast<size_t>(1));
                        remaining_tasks -= n_tasks;
                    }

                    for (size_t j = 0; j < n_tasks; j++) {
                        queue.add_task([this, &counter, &partitions, i, j, n_tasks](size_t) {
                            const size_t length = partitions.histogram[i];
                            const T *begin = partitions.values + partitions.offsets[i] +
                                             simd::valign<Vector>((length / n_tasks + 1) * j);
                            const T *end = partitions.values + partitions.offsets[i] +
                                           std::min(simd::valign<Vector>((length / n_tasks + 1) * (j + 1)), length);

                            counter += container.template count<nxor::RF0, nxor::RF1, nxor::RF2>(begin, end, i);
                        });
                    }
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() {
            return container.length();
        }

        size_t avg_size() {
            return size() / n_partitions;
        }

        size_t retries() {
            return n_retries;
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(k) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"n_partitions\": " + std::to_string(n_partitions) + ",\n";
            s += "\t\"n_retries\": " + std::to_string(n_retries) + ",\n";
            s += "\t\"filter_params\": " + FP::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters