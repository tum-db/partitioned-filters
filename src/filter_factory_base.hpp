#pragma once

#include <parameter/parameter.hpp>
#include <stdexcept>
#include <simd/vector.hpp>
#include <functional>
#include "filter_base.hpp"

namespace filters {

    template<FilterType type, typename OP>
    struct FilterFactory {

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;

        using f_ctor = std::function<void *(size_t /*s*/)>;
        using f_dtor = std::function<void(void */*instance*/)>;
        using f_add_all = std::function<bool(void */*instance*/, const T * /*values*/, const size_t /*length*/)>;
        using f_contains = std::function<bool(const void */*instance*/, const T /*value*/)>;

        static size_t optimal_k(size_t /*n*/, size_t /*m*/) {
            throw std::runtime_error{"not implemented!"};
        }

        static double optimal_fpr(size_t /*k*/, size_t /*n*/, size_t /*m*/) {
            throw std::runtime_error{"not implemented!"};
        }

        static size_t optimal_m(size_t /*k*/, size_t /*n*/, size_t /*m*/) {
            throw std::runtime_error{"not implemented!"};
        }

        template<size_t /*k*/>
        static void bind(f_ctor &ctor, f_dtor &dtor, f_add_all &add_all, f_contains &contains) {
            throw std::runtime_error{"not implemented!"};
        }

    };

} // filters
