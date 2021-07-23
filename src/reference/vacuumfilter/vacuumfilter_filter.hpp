#pragma once

#include <filter_base.hpp>
#include <memory>
#include <vacuumfilter/ModifiedCuckooFilter/src/cuckoofilter.h>
#include <task/task_queue.hpp>

namespace filters {

    template<typename FP, size_t k, typename OP>
    struct Filter<FilterType::VacuumFilter, FP, k, OP> {

        static constexpr bool supports_add = true;
        static constexpr bool supports_add_partition = false;

        static_assert(OP::simd == parameter::SIMD::Scalar, "only Scalar is supported!");
        static_assert(OP::partitioning == parameter::Partitioning::Disabled, "partitioning must be disabled!");
        static_assert(OP::hashingMode == parameter::HashingMode::TwoIndependentMultiplyShift or
                      OP::hashingMode == parameter::HashingMode::SimpleTabulation,
                "only TwoIndependentMultiplyShift and SimpleTabulation are supported!");
        static_assert(OP::addressingMode == parameter::AddressingMode::Vacuum, "only Vacuum is supported!");

        static constexpr bool semiSorted = FP::semiSorted;

        // not semiSorted -> k in {2, 4, 8, 12, 16}
        static_assert(semiSorted or k == 2 or k == 4 or k == 8 or k == 12 or k == 16, "wrong k!");

        // semiSorted -> k in {5, 6, 7, 8, 9, 13, 17}
        static_assert(not semiSorted or k == 5 or k == 6 or k == 7 or k == 8 or k == 9 or k == 13 or k == 17,
                "wrong k!");

        using T = std::conditional_t<OP::registerSize == parameter::RegisterSize::_32bit, uint32_t, uint64_t>;
        using H = std::conditional_t<OP::hashingMode == parameter::HashingMode::TwoIndependentMultiplyShift,
                modifiedcuckoofilter::TwoIndependentMultiplyShift, modifiedcuckoofilter::SimpleTabulation>;
        using F = std::conditional_t<semiSorted,
                modifiedcuckoofilter::VacuumFilter<T, k, modifiedcuckoofilter::PackedTable, H>,
                modifiedcuckoofilter::VacuumFilter<T, k, modifiedcuckoofilter::SingleTable, H>>;

        size_t n_partitions = 1;
        size_t s;
        std::unique_ptr<F> filter;
        task::TaskQueue <OP::multiThreading> queue;

        Filter(size_t s, size_t, size_t n_threads, size_t n_tasks_per_level) : s(s), n_partitions(n_partitions),
                                                                               queue(n_threads, n_tasks_per_level) {
        }

        forceinline
        void init(const T *histogram) {
            // multiply with 0.96 since VacuumFilter implementation divides max elements by 0.96
            filter = std::make_unique<F>(*histogram * s / 100 * 0.96);
        }

        forceinline
        bool contains(const T &value, const size_t = 0) {
            return modifiedcuckoofilter::Status::Ok == filter->Contain(value);
        }

        forceinline
        bool add(const T &value, const size_t = 0) {
            return modifiedcuckoofilter::Status::Ok == filter->Add(value);
        }

        bool construct(T *values, size_t length) {
            T histogram = length;
            init(&histogram);
            for (size_t i = 0; i < length; i++) {
                filter->Add(values[i]);
            }
            return (filter->Size() == length);
        }

        size_t count(T *values, size_t length) {
            if constexpr (OP::multiThreading == parameter::MultiThreading::Disabled) {
                size_t counter = 0;
                for (size_t i = 0; i < length; i++) {
                    counter += contains(values[i]);
                }
                return counter;
            } else {
                std::atomic<size_t> counter{0};

                size_t begin = 0;
                for (size_t i = queue.get_n_tasks_per_level(); i > 0; i--) {
                    const size_t end = begin + (length - begin) / i;
                    queue.add_task([this, &counter, values, begin, end](size_t) {
                        size_t local_counter = 0;
                        for (size_t i = begin; i < end; i++) {
                            local_counter += contains(values[i]);
                        }
                        counter += local_counter;
                    });
                    begin = end;
                }
                queue.execute_tasks();

                return counter;
            }
        }

        size_t size() {
            return filter->SizeInBytes();
        }

        size_t avg_size() {
            // no partitioning available
            return size();
        }

        size_t retries() {
            return 0;
        }

        std::string to_string() {
            std::string s = "\n{\n";
            s += "\t\"k\": " + std::to_string(k) + ",\n";
            s += "\t\"size\": " + std::to_string(size() * 8) + " bits,\n";
            s += "\t\"filter_params\": " + FP::to_string() + ",\n";
            s += "\t\"optimization_params\": " + OP::to_string() + "\n";
            s += "}\n";

            return s;
        }
    };

} // filters
