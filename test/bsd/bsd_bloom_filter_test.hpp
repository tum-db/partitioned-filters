#pragma once

#include <cstddef>
#include <reference/bsd/bloom/bsd_bloom_parameter.hpp>
#include <reference/bsd/bloom/bsd_bloom_filter.hpp>
#include "../filter_test.hpp"

namespace test::bsd::bloom {

    namespace bloom = filters::bsd::bloom;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 144;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k_s = 8, k_l = 16;

    static constexpr filters::FilterType BSDBloom = filters::FilterType::BSDBloom;


    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarSmall1 = FilterTestConfig<BSDBloom, FP,
            k_l, parameter::MagicMurmurScalar32, n_s, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarSmall2 = FilterTestConfig<BSDBloom, FP,
            k_l, parameter::PowerOfTwoMulScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    #ifdef __AVX512F__
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVXSmall = FilterTestConfig<BSDBloom, FP, k_l,
            parameter::PowerOfTwoMulAVX51232, n_s, s, 0, 0, 0, expected_fp>;
    #else
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVXSmall = FilterTestConfig<BSDBloom, FP,
            k_l, parameter::PowerOfTwoMulAVX232, n_s, s, 0, 0, 0, expected_fp>;
    #endif

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarLarge = FilterTestConfig<BSDBloom, FP,
            k_s, parameter::PowerOfTwoMulScalar32, n_l, s, 0, 4, 16, expected_fp>;
    #ifdef __AVX512F__
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVXLarge = FilterTestConfig<BSDBloom, FP, k_s,
            parameter::MagicMurmurAVX51232MT, n_l, s, 0, 4, 16, expected_fp>;
    #else
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVXLarge = FilterTestConfig<BSDBloom, FP,
            k_s, parameter::MagicMurmurAVX232MT, n_l, s, 0, 4, 16, expected_fp>;
    #endif

    /*
     * Variant Test Types
     */

    using BSDBloomNaive32TestTypes = ::testing::Types<BloomScalarSmall2<bloom::Naive32, 0>,
            BloomScalarSmall2<bloom::Naive32Batched, 0>, BloomScalarLarge<bloom::Naive32, 4002>,
            BloomScalarLarge<bloom::Naive32Batched, 4002>>;

    using BSDBloomBlocked32TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Blocked32, 120>,
            BloomScalarSmall2<bloom::Blocked32, 43>, BloomAVXSmall<bloom::Blocked32, 43>,
            BloomAVXSmall<bloom::Blocked32Batched, 43>, BloomScalarLarge<bloom::Blocked32, 75878>,
            BloomAVXLarge<bloom::Blocked32, 278322>, BloomAVXLarge<bloom::Blocked32Batched, 278322>>;

    using BSDBloomBlocked256TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Blocked256, 1>,
            BloomScalarSmall2<bloom::Blocked256, 0>, BloomAVXSmall<bloom::Blocked256, 0>,
            BloomAVXSmall<bloom::Blocked256Batched, 0>, BloomScalarLarge<bloom::Blocked256, 9295>,
            BloomAVXLarge<bloom::Blocked256, 63247>, BloomAVXLarge<bloom::Blocked256Batched, 63247>>;

    using BSDBloomBlocked512TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Blocked512, 1>,
            BloomScalarSmall2<bloom::Blocked512, 0>, BloomAVXSmall<bloom::Blocked512, 0>,
            BloomAVXSmall<bloom::Blocked512Batched, 0>, BloomScalarLarge<bloom::Blocked512, 6933>,
            BloomAVXLarge<bloom::Blocked512, 50828>, BloomAVXLarge<bloom::Blocked512Batched, 50828>>;

    using BSDBloomSectorized256TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Sectorized256, 1>,
            BloomScalarSmall2<bloom::Sectorized256, 0>, BloomAVXSmall<bloom::Sectorized256, 0>,
            BloomAVXSmall<bloom::Sectorized256Batched, 0>, BloomScalarLarge<bloom::Sectorized256, 4892>,
            BloomAVXLarge<bloom::Sectorized256, 65531>, BloomAVXLarge<bloom::Sectorized256Batched, 65531>>;

    using BSDBloomSectorized512TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Sectorized512, 0>,
            BloomScalarSmall2<bloom::Sectorized512, 0>, BloomAVXSmall<bloom::Sectorized512, 0>,
            BloomAVXSmall<bloom::Sectorized512Batched, 0>>;

    using BSDBloomGrouped2TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Grouped2, 4>,
            BloomScalarSmall2<bloom::Grouped2, 9>, BloomAVXSmall<bloom::Grouped2, 9>,
            BloomAVXSmall<bloom::Grouped2Batched, 9>, BloomScalarLarge<bloom::Grouped2, 16271>,
            BloomAVXLarge<bloom::Grouped2, 87443>, BloomAVXLarge<bloom::Grouped2Batched, 87443>>;

    using BSDBloomGrouped4TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Grouped4, 3>,
            BloomScalarSmall2<bloom::Grouped4, 1>, BloomAVXSmall<bloom::Grouped4, 1>,
            BloomAVXSmall<bloom::Grouped4Batched, 1>, BloomScalarLarge<bloom::Grouped4, 7606>,
            BloomAVXLarge<bloom::Grouped4, 55586>, BloomAVXLarge<bloom::Grouped4Batched, 55586>>;

    using BSDBloomGrouped8TestTypes = ::testing::Types<BloomScalarSmall1<bloom::Grouped8, 0>,
            BloomScalarSmall2<bloom::Grouped8, 0>, BloomAVXSmall<bloom::Grouped8, 0>,
            BloomAVXSmall<bloom::Grouped8Batched, 0>, BloomScalarLarge<bloom::Grouped8, 5746>,
            BloomAVXLarge<bloom::Grouped8, 52160>, BloomAVXLarge<bloom::Grouped8Batched, 52160>>;

}
