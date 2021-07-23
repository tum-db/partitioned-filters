#pragma once

#include <parameter/parameter.hpp>
#include <stdexcept>
#include <simd/vector.hpp>

namespace filters {

    enum class FilterType : size_t {
        Bloom,
        Xor,
        Cuckoo,
        FastfilterBloom,
        FastfilterXor,
        ImpalaBloom,
        BSDBloom,
        BSDCuckoo,
        EfficientCuckooFilter,
        AMDMortonFilter,
        VacuumFilter
    };

    template<FilterType filter, typename FilterParameter, size_t k, typename OptimizationParameter>
    struct Filter {

        static constexpr bool supports_add = false;
        static constexpr bool supports_add_partition = false;

        using Vector = simd::Vector<OptimizationParameter::registerSize, OptimizationParameter::simd>;
        using T = typename Vector::T;
        using M = typename Vector::M;

        Filter(size_t /*s*/, size_t /*n_partitions*/, size_t /*n_threads*/, size_t /*n_tasks_per_level*/) {
            throw std::logic_error{"not implemented!"};
        }

        void init(const T *) {
            throw std::logic_error{"not implemented!"};
        }

        bool contains(const T &) {
            throw std::logic_error{"not implemented!"};
        }

        bool add(const T &) {
            throw std::logic_error{"not implemented!"};
        }

        bool construct(T *, size_t) {
            throw std::logic_error{"not implemented!"};
        }

        size_t count(T *, size_t) {
            throw std::logic_error{"not implemented!"};
        }

        size_t size() {
            throw std::logic_error{"not implemented!"};
        }

        size_t avg_size() {
            throw std::logic_error{"not implemented!"};
        };

        size_t retries() {
            throw std::logic_error{"not implemented"};
        }

        std::string to_string() {
            throw std::logic_error{"not implemented!"};
        }
    };

} // filters
