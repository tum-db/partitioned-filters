#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::bsd::cuckoo {

    namespace parameter = filters::parameter;
    namespace cuckoo = filters::bsd::cuckoo;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr filters::FilterType Cuckoo = filters::FilterType::BSDCuckoo;

    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarSmall = FilterTestConfig<
            Cuckoo, FP, 12, parameter::MagicMulScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    #ifdef __AVX512F__
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVXSmall = FilterTestConfig<Cuckoo, FP, 16,
            parameter::PowerOfTwoMulAVX51232, n_s, s, 0, 0, 0, expected_fp>;
    #else
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVXSmall = FilterTestConfig<
            Cuckoo, FP, 16, parameter::PowerOfTwoMulAVX232, n_s, s, 0, 0, 0, expected_fp>;
    #endif

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarLarge = FilterTestConfig<
            Cuckoo, FP, 16, parameter::PowerOfTwoMulScalar32, n_l, s, 0, 0, 0, expected_fp>;
    #ifdef __AVX512F__
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVXLarge = FilterTestConfig<Cuckoo, FP, 8,
                parameter::MagicMulAVX51232MT, n_l, s, 0, 4, 16, expected_fp>;
    #else
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVXLarge = FilterTestConfig<
            Cuckoo, FP, 8, parameter::MagicMulAVX232MT, n_l, s, 0, 4, 16, expected_fp>;
    #endif



    /*
     * Variant Test Types
     */

    using BSDCuckooStandard2TestTypes = ::testing::Types<CuckooScalarSmall<cuckoo::Standard2, 140, 6>,
            CuckooAVXSmall<cuckoo::Standard2, 105, 0>, CuckooScalarLarge<cuckoo::Standard2, 105, 158>,
            CuckooAVXLarge<cuckoo::Standard2, 205, 78311>>;

    using BSDCuckooStandard4TestTypes = ::testing::Types<CuckooScalarSmall<cuckoo::Standard4, 120, 7>,
            CuckooAVXSmall<cuckoo::Standard4, 105, 0>, CuckooScalarLarge<cuckoo::Standard4, 100, 561>,
            CuckooAVXLarge<cuckoo::Standard4, 125, 597236>>;

}


