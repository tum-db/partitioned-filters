#pragma once

#include <parameter/parameter.hpp>
#include <stdexcept>
#include <simd/vector.hpp>
#include <functional>
#include <filter_factory_base.hpp>
#include <cmath>
#include "filter_base.hpp"
#include "cuckoo_parameter.hpp"

namespace filters {

    template<typename OP>
    struct FilterFactory<FilterType::Cuckoo, OP> {

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;

        using f_ctor = std::function<void *(size_t /*s*/)>;
        using f_dtor = std::function<void(void */*instance*/)>;
        using f_add_all = std::function<bool(void */*instance*/, const T * /*values*/, const size_t /*length*/)>;
        using f_contains = std::function<bool(const void */*instance*/, const T /*value*/)>;

        static size_t optimal_k(size_t n, size_t m) {
            // 0.87 is lowest load factor used (1 / 1.15), k is an upper estimate at first
            size_t k = std::min(32, static_cast<int>(static_cast<double>(m) / static_cast<double>(n) / 1.15));
            // reduce k until filter is smaller than expected size
            for (; k > 0 and optimal_m(k, n, m) > m; k--);
            return k;
        }

        static double exact_k(size_t n, size_t m) {
            return static_cast<double>(m) / static_cast<double>(n) / 1.15;
        }

        static double optimal_fpr(size_t k, size_t /*n*/, size_t /*m*/) {
            if (k == 0) {
                return 1.0;
            }
            const size_t associativity = cuckoo::optimal_associativity(k, cuckoo::Variant::CuckooOpt, 0);
            const double load_factor = cuckoo::optimal_load_factor(k);
            return 1.0 - std::pow(1.0 - 1.0 / std::pow(2, k), 2 * associativity / load_factor);
        }

        static double exact_fpr(double k, size_t /*n*/, size_t /*m*/) {
            return 1.0 - std::pow(1.0 - 1.0 / std::pow(2, k), 2 * 2 / 1.15);
        }

        static size_t optimal_m(size_t k, size_t n, size_t /*m*/) {
            if (k == 0) {
                return 0;
            }
            const size_t associativity = cuckoo::optimal_associativity(k, cuckoo::Variant::CuckooOpt, 0);
            const double load_factor = cuckoo::optimal_load_factor(k);
            const size_t bucket_size = cuckoo::optimal_bucket_size(k, associativity);
            const size_t n_buckets = std::ceil(
                    static_cast<double>((n + associativity - 1) / associativity) * load_factor);
            return n_buckets * bucket_size;
        }

        template<size_t k>
        static void bind(f_ctor &ctor, f_dtor &dtor, f_add_all &add_all, f_contains &contains) {
            using FP = cuckoo::CuckooFilterParameter<k, cuckoo::Variant::CuckooOpt, 0, 0, 0>;
            using F = Filter<FilterType::Cuckoo, FP, k, OP>;

            ctor = [](size_t s) -> void * {
                return new F(s, 0, 0, 0);
            };
            dtor = [](void *instance) {
                delete reinterpret_cast<F *>(instance);
            };
            add_all = [](void *instance, const T *values, const size_t length) -> bool {
                T histogram = length;
                reinterpret_cast<F *>(instance)->init(&histogram);
                return reinterpret_cast<F *>(instance)->add_all(values, length);
            };
            contains = [](const void *instance, const T value) -> bool {
                return reinterpret_cast<const F *>(instance)->contains(value);
            };
        }
    };

} // filters
