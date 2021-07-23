#pragma once

#include <cstddef>
#include <reference/vacuumfilter/vacuumfilter_parameter.hpp>
#include "../filter_test.hpp"

namespace test::vacuumfilter {

    namespace parameter = filters::parameter;
    namespace vacuum = filters::vacuumfilter;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr filters::FilterType Vacuum = filters::FilterType::VacuumFilter;

    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using VacuumScalarSmall = FilterTestConfig<
            Vacuum, FP, k, parameter::VacuumTwoIndependentMultiplyShiftScalar64, n_s, 105, 0, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using VacuumScalarLarge1 = FilterTestConfig<
            Vacuum, FP, k, parameter::VacuumTwoIndependentMultiplyShiftScalar32MT, n_l, 110, 0, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using VacuumScalarLarge2 = FilterTestConfig<
            Vacuum, FP, k, parameter::VacuumSimpleTabulationScalar32, n_l, 105, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t k, int64_t expected_fp> using VacuumScalarLarge3 = FilterTestConfig<
            Vacuum, FP, k, parameter::VacuumSimpleTabulationScalar64, n_l, 105, 0, 0, 0, expected_fp>;


    /*
     * Variant Test Types
     */

    using VacuumFilterStandardTestTypes = ::testing::Types<VacuumScalarSmall<vacuum::Standard, 16, 0>,
            VacuumScalarLarge1<vacuum::Standard, 8, 308708>, VacuumScalarLarge2<vacuum::Standard, 4, 3906277>,
            VacuumScalarLarge3<vacuum::Standard, 12, 17292>>;

    using VacuumFilterSemiSortedTestTypes = ::testing::Types<VacuumScalarSmall<vacuum::SemiSorted, 5, 591>,
            VacuumScalarLarge1<vacuum::SemiSorted, 6, 1086289>, VacuumScalarLarge2<vacuum::SemiSorted, 7, 553483>,
            VacuumScalarLarge3<vacuum::SemiSorted, 8, 277968>, VacuumScalarLarge1<vacuum::SemiSorted, 9, 208550>,
            VacuumScalarLarge2<vacuum::SemiSorted, 13, 8778>, VacuumScalarLarge3<vacuum::SemiSorted, 17, 547>>;

}
