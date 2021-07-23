#pragma once

#include <parameter/parameter.hpp>
#include <stdexcept>
#include <simd/vector.hpp>
#include <functional>
#include <filter_factory_base.hpp>
#include "filter_base.hpp"
#include "xor_parameter.hpp"

namespace filters {

    template<typename OP>
    struct FilterFactory<FilterType::Xor, OP> {

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;

        using f_ctor = std::function<void *(size_t /*s*/)>;
        using f_dtor = std::function<void(void */*instance*/)>;
        using f_add_all = std::function<bool(void */*instance*/, const T * /*values*/, const size_t /*length*/)>;
        using f_contains = std::function<bool(const void */*instance*/, const T /*value*/)>;

        static size_t optimal_k(size_t n, size_t m) {
            // k is an upper estimate at first
            size_t k = std::min(32, static_cast<int>(static_cast<double>(m) / static_cast<double>(n) / 1.25));
            // reduce k until filter is smaller than expected size
            for (; k > 0 and optimal_m(k, n, m) > m; k--);
            return k;
        }

        static double exact_k(size_t n, size_t m) {
            return static_cast<double>(m) / static_cast<double>(n) / 1.25;
        }

        static double optimal_fpr(size_t k, size_t /*n*/, size_t /*m*/) {
            return 1.0 / std::pow(2, k);
        }

        static double exact_fpr(double k, size_t /*n*/, size_t /*m*/) {
            return 1.0 / std::pow(2, k);
        }

        static size_t optimal_m(size_t k, size_t n, size_t /*m*/) {
            return math::align_number(math::align_number(n * 125, 300ul) / 100 * k, 8ul);
        }

        template<size_t k>
        static void bind(f_ctor &ctor, f_dtor &dtor, f_add_all &add_all, f_contains &contains) {
            using F = Filter<FilterType::Xor, nxor::Standard<k>, k, OP>;

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
