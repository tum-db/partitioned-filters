#pragma once

#include <cstddef>
#include <reference/amd_mortonfilter/amd_mortonfilter_parameter.hpp>
#include "../filter_test.hpp"

namespace test::amdmortonfilter {

    namespace parameter = filters::parameter;
    namespace amdmorton = filters::amdmortonfilter;

    static constexpr size_t n_l = 1000000;

    static constexpr filters::FilterType AMDMorton = filters::FilterType::AMDMortonFilter;

    /*
     * Small Test Types
     */

    // will cause segfaults

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using AMDMortonScalarLarge1 = FilterTestConfig<AMDMorton, FP,
            12, parameter::LemireMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using AMDMortonScalarLarge2 = FilterTestConfig<AMDMorton, FP,
            8, parameter::LemireMurmurScalar64MT, n_l, s, 0, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using AMDMortonScalarLarge3 = FilterTestConfig<AMDMorton, FP,
            6, parameter::LemireMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;


    /*
     * Variant Test Types
     */

    using AMDMortonfilterMorton1TestTypes = ::testing::Types<AMDMortonScalarLarge2<amdmorton::Standard1, 150, -1>>;

    using AMDMortonfilterMorton3TestTypes = ::testing::Types<AMDMortonScalarLarge1<amdmorton::Standard3, 101, -1>,
            AMDMortonScalarLarge2<amdmorton::Standard3, 102, -1>, AMDMortonScalarLarge3<amdmorton::Standard3, 102, -1>>;

    using AMDMortonfilterMorton7TestTypes = ::testing::Types<AMDMortonScalarLarge1<amdmorton::Standard7, 102, -1>,
            AMDMortonScalarLarge2<amdmorton::Standard7, 101, -1>, AMDMortonScalarLarge3<amdmorton::Standard7, 102, -1>>;

    using AMDMortonfilterMorton15TestTypes = ::testing::Types<AMDMortonScalarLarge1<amdmorton::Standard15, 101, -1>,
            AMDMortonScalarLarge2<amdmorton::Standard15, 101, -1>,
            AMDMortonScalarLarge3<amdmorton::Standard15, 102, -1>>;

}
