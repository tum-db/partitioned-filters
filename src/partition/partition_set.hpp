#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <simd/helper.hpp>
#include <math/math.hpp>
#include "cache_line_buffer.hpp"

namespace filters::partition {

    template<parameter::Partitioning, typename>
    struct PartitionSet {
    };

    template<typename Vector>
    struct PartitionSet<parameter::Partitioning::Disabled, Vector> {
        using T = typename Vector::T;

        size_t n_partitions;

        size_t length;
        T *values;

        T *histogram;
        T *offsets;

        PartitionSet(size_t) : n_partitions(1) {
            size_t n_bytes = simd::valign<Vector>(sizeof(T));
            histogram = simd::valloc<Vector>(n_bytes, 0);
            offsets = simd::valloc<Vector>(n_bytes, 0);
        }

        forceinline
        void init(T *v, size_t l) {
            this->length = l;
            this->values = v;
            histogram[0] = l;
        }

        template<typename TaskQueue>
        forceinline
        void init(T *v, size_t l, TaskQueue &) {
            init(v, l);
        }

        ~PartitionSet() {
            free(histogram);
            free(offsets);
        }
    };

    template<typename Vector>
    struct PartitionSet<parameter::Partitioning::Enabled, Vector> {
        using T = typename Vector::T;
        using CLB = CacheLineBuffer<T>;

        size_t n_partitions;

        size_t length;
        T *values;

        T *histogram;
        T *offsets;

        PartitionSet(size_t n_partitions) : n_partitions(n_partitions), values(nullptr) {
            size_t n_bytes = simd::valign<Vector>(n_partitions * sizeof(T));
            histogram = simd::valloc<Vector>(n_bytes, 0);
            offsets = simd::valloc<Vector>(n_bytes, 0);
        }

        forceinline
        void allocate(size_t n_elements) {
            length = sizeof(T) * n_elements;
            values = reinterpret_cast<T *>(aligned_alloc(64, length));
        }

        forceinline
        void init(T *v, size_t l) {
            size_t partitioning_mask = 0;
            if (n_partitions > 1) {
                partitioning_mask = (1ull << math::get_number_of_bits(n_partitions)) - 1;
            }

            for (size_t i = 0; i < l; i++) {
                auto bucket = v[i] & partitioning_mask;
                histogram[bucket]++;
            }

            T last_offset = 0;
            for (size_t partition = 0; partition < n_partitions; partition++) {
                offsets[partition] = last_offset;
                last_offset += (histogram[partition] / CLB::n_tuples_per_cache_line_buffer + 1) *
                               CLB::n_tuples_per_cache_line_buffer;
            }
            allocate(last_offset);

            T *start_pos[n_partitions];
            size_t buffer_offset[n_partitions];
            for (size_t partition = 0; partition < n_partitions; partition++) {
                start_pos[partition] = values + offsets[partition];
                buffer_offset[partition] = 0;
            }

            CLB *cacheLineBuffer = new CLB[n_partitions];
            for (size_t i = 0; i < l; i++) {
                auto partition = v[i] & partitioning_mask;
                cacheLineBuffer[partition].tuples[buffer_offset[partition]++] = v[i];

                if (buffer_offset[partition] == CLB::n_tuples_per_cache_line_buffer) {
                    cacheLineBuffer[partition].store_non_temporal(start_pos[partition]);
                    start_pos[partition] += CLB::n_tuples_per_cache_line_buffer;
                    buffer_offset[partition] = 0;
                }
            }

            for (size_t i = 0; i < n_partitions; i++) {
                if (buffer_offset[i] != 0) {
                    cacheLineBuffer[i].store_non_temporal(start_pos[i]);
                }
            }

            delete[](cacheLineBuffer);
        }

        template<typename TaskQueue>
        forceinline
        void init(T *v, size_t l, TaskQueue &queue) {
            size_t partitioning_mask = 0;
            if (n_partitions > 1) {
                partitioning_mask = (1ull << math::get_number_of_bits(n_partitions)) - 1;
            }

            T **local_histograms = new T *[queue.get_n_tasks_per_level()];
            T **local_offsets = new T *[queue.get_n_tasks_per_level()];

            CLB **cacheLineBuffers = new CLB *[queue.get_n_threads()];
            T **buffer_offsets = new T *[queue.get_n_threads()];
            for (size_t i = 0; i < queue.get_n_threads(); i++) {
                cacheLineBuffers[i] = new CLB[n_partitions];
                buffer_offsets[i] = new T[n_partitions];
                std::memset(buffer_offsets[i], 0, n_partitions * sizeof(T));
            }

            for (size_t i = 0, begin = 0; i < queue.get_n_tasks_per_level(); i++) {
                size_t end = begin + (l - begin) / (queue.get_n_tasks_per_level() - i);
                queue.add_task([this, i, begin, end, local_histograms, local_offsets, v, partitioning_mask](size_t) {
                    local_histograms[i] = new T[n_partitions];
                    local_offsets[i] = new T[n_partitions];
                    std::memset(local_histograms[i], 0, n_partitions * sizeof(T));

                    for (T *iterator = v + begin; iterator < v + end; iterator++) {
                        auto bucket = *iterator & partitioning_mask;
                        local_histograms[i][bucket]++;
                    }
                });
                begin = end;
            }

            queue.add_barrier();

            queue.add_task([this, &queue, local_histograms, local_offsets](size_t) {
                size_t last_offset = 0;
                for (size_t partition = 0; partition < n_partitions; partition++) {
                    offsets[partition] = last_offset;

                    size_t counter = 0;
                    for (size_t task = 0; task < queue.get_n_tasks_per_level(); task++) {
                        local_offsets[task][partition] = last_offset + counter;
                        counter += local_histograms[task][partition];
                    }

                    histogram[partition] = counter;
                    last_offset +=
                            (counter / CLB::n_tuples_per_cache_line_buffer + 1) * CLB::n_tuples_per_cache_line_buffer;
                }

                allocate(last_offset);
            });

            queue.add_barrier();

            for (size_t i = 0, begin = 0; i < queue.get_n_tasks_per_level(); i++) {
                size_t end = begin + (l - begin) / (queue.get_n_tasks_per_level() - i);
                queue.add_task(
                        [this, i, begin, end, cacheLineBuffers, buffer_offsets, local_offsets, partitioning_mask, v, local_histograms](
                                size_t thread) {

                            for (T *iterator = v + begin; iterator < v + end; iterator++) {
                                auto partition = *iterator & partitioning_mask;
                                cacheLineBuffers[thread][partition].tuples[buffer_offsets[thread][partition]++] = *iterator;

                                if (buffer_offsets[thread][partition] == CLB::n_tuples_per_cache_line_buffer) {
                                    cacheLineBuffers[thread][partition].store(values + local_offsets[i][partition],
                                            CLB::n_tuples_per_cache_line_buffer);
                                    local_offsets[i][partition] += CLB::n_tuples_per_cache_line_buffer;
                                    buffer_offsets[thread][partition] = 0;
                                }
                            }

                            for (size_t partition = 0; partition < n_partitions; partition++) {
                                cacheLineBuffers[thread][partition].store(values + local_offsets[i][partition],
                                        buffer_offsets[thread][partition]);
                                buffer_offsets[thread][partition] = 0;
                            }

                            delete[](local_histograms[i]);
                            delete[](local_offsets[i]);
                        });

                begin = end;
            }

            queue.add_barrier();

            queue.add_task([&queue, cacheLineBuffers, buffer_offsets, local_histograms, local_offsets](size_t) {
                for (size_t i = 0; i < queue.get_n_threads(); i++) {
                    delete[](cacheLineBuffers[i]);
                    delete[](buffer_offsets[i]);
                }

                delete[](local_histograms);
                delete[](local_offsets);
                delete[](cacheLineBuffers);
                delete[](buffer_offsets);
            });
        }

        ~PartitionSet() {
            if (values) {
                free(values);
            }
            free(histogram);
            free(offsets);
        }
    };
}