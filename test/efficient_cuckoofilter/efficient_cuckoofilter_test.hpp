#pragma once

#include <cstddef>
#include <reference/efficient_cuckoofilter/efficient_cuckoofilter_parameter.hpp>
#include "../filter_test.hpp"

namespace test::efficientcuckoofilter {

    namespace parameter = filters::parameter;
    namespace ecuckoo = filters::efficientcuckoofilter;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr filters::FilterType ECuckoo = filters::FilterType::EfficientCuckooFilter;

    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using ECuckooScalarSmall = FilterTestConfig<
            ECuckoo, FP, k, parameter::PowerOfTwoTwoIndependentMultiplyShiftScalar64, n_s, 110, 0, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using ECuckooScalarLarge1 = FilterTestConfig<
            ECuckoo, FP, k, parameter::PowerOfTwoTwoIndependentMultiplyShiftScalar32MT, n_l, 110, 0, 4, 16,
            expected_fp>;
    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using ECuckooScalarLarge2 = FilterTestConfig<
            ECuckoo, FP, k, parameter::PowerOfTwoSimpleTabulationScalar32, n_l, 110, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using ECuckooScalarLarge3 = FilterTestConfig<
            ECuckoo, FP, k, parameter::PowerOfTwoSimpleTabulationScalar64, n_l, 110, 0, 0, 0, expected_fp>;


    /*
     * Variant Test Types
     */

    using EfficientCuckoofilterStandardTestTypes = ::testing::Types<ECuckooScalarSmall<ecuckoo::Standard, 16, -1>,
            ECuckooScalarLarge1<ecuckoo::Standard, 8, -1>, ECuckooScalarLarge2<ecuckoo::Standard, 4, -1>,
            ECuckooScalarLarge3<ecuckoo::Standard, 12, -1>>;

    using EfficientCuckoofilterOptimizedTestTypes = ::testing::Types<ECuckooScalarSmall<ecuckoo::SemiSorted, 13, -1>,
            ECuckooScalarLarge1<ecuckoo::SemiSorted, 13, -1>, ECuckooScalarLarge2<ecuckoo::SemiSorted, 13, -1>,
            ECuckooScalarLarge3<ecuckoo::SemiSorted, 13, -1>>;

}
