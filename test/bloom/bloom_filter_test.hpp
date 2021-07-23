#pragma once

#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include "../filter_test.hpp"

namespace test::bloom {

    namespace bloom = filters::bloom;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 144;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k_s = 8, k_l = 16;

    static constexpr size_t n_partitions_s = 2, n_partitition_l = 16;

    static constexpr filters::FilterType Bloom = filters::FilterType::Bloom;


    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarSmall = FilterTestConfig<Bloom, FP,
            k_l, parameter::MagicMurmurScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVXSmall1 = FilterTestConfig<Bloom, FP, k_l,
            parameter::LemireMurmurAVX232Partitioned, n_s, s, n_partitions_s, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVXSmall2 = FilterTestConfig<Bloom, FP, k_l,
            parameter::PowerOfTwoMurmurAVX51264Partitioned, n_s, s, n_partitition_l, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarLarge1 = FilterTestConfig<Bloom, FP,
            k_s, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarLarge2 = FilterTestConfig<Bloom, FP,
            k_s, parameter::MagicMurmurScalar32Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomScalarLarge3 = FilterTestConfig<Bloom, FP,
            k_s, parameter::LemireMurmurScalar32MT, n_l, s, 0, 4, 16, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVX2Large1 = FilterTestConfig<Bloom, FP, k_s,
            parameter::PowerOfTwoMurmurAVX264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVX2Large2 = FilterTestConfig<Bloom, FP, k_s,
            parameter::MagicMurmurAVX232PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVX2Large3 = FilterTestConfig<Bloom, FP, k_s,
            parameter::LemireMurmurAVX232Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVX512Large1 = FilterTestConfig<Bloom, FP,
            k_s, parameter::PowerOfTwoMurmurAVX51232PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVX512Large2 = FilterTestConfig<Bloom, FP,
            k_s, parameter::MagicMurmurAVX51264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, int64_t expected_fp> using BloomAVX512Large3 = FilterTestConfig<Bloom, FP,
            k_s, parameter::LemireMurmurAVX51264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;


    /*
     * Variant Test Types
     */

    using BloomNaive32TestTypes = ::testing::Types<BloomScalarSmall<bloom::Naive32, 0>,
            BloomAVXSmall1<bloom::Naive32, 0>, BloomScalarLarge2<bloom::Naive32, 39652>,
            BloomScalarLarge3<bloom::Naive32, 39318>, BloomAVX2Large2<bloom::Naive32, 39652>,
            BloomAVX2Large3<bloom::Naive32, 39678>, BloomAVX512Large1<bloom::Naive32, 4269>>;

    using BloomNaive64TestTypes = ::testing::Types<BloomAVXSmall2<bloom::Naive64, 0>,
            BloomScalarLarge1<bloom::Naive64, 4233>, BloomAVX2Large1<bloom::Naive64, 4289>,
            BloomAVX512Large2<bloom::Naive64, 39266>, BloomAVX512Large3<bloom::Naive64, 39660>>;

    using BloomBlocked32TestTypes = ::testing::Types<BloomScalarSmall<bloom::Blocked32, 196>,
            BloomAVXSmall1<bloom::Blocked32, 136>, BloomScalarLarge2<bloom::Blocked32, 278004>,
            BloomScalarLarge3<bloom::Blocked32, 279132>, BloomAVX2Large2<bloom::Blocked32, 278004>,
            BloomAVX2Large3<bloom::Blocked32, 276696>, BloomAVX512Large1<bloom::Blocked32, 95132>>;

    using BloomBlocked64TestTypes = ::testing::Types<BloomAVXSmall2<bloom::Blocked64, 10>,
            BloomScalarLarge1<bloom::Blocked64, 37730>, BloomAVX2Large1<bloom::Blocked64, 37560>,
            BloomAVX512Large2<bloom::Blocked64, 147868>, BloomAVX512Large3<bloom::Blocked64, 147422>>;

    using BloomBlocked256TestTypes = ::testing::Types<BloomScalarSmall<bloom::Blocked256, 0>,
            BloomAVXSmall1<bloom::Blocked256, 1>, BloomAVXSmall2<bloom::Blocked256, 0>,
            BloomScalarLarge1<bloom::Blocked256, 9887>, BloomScalarLarge2<bloom::Blocked256, 62816>,
            BloomScalarLarge3<bloom::Blocked256, 63291>, BloomAVX2Large1<bloom::Blocked256, 9938>,
            BloomAVX2Large2<bloom::Blocked256, 62816>, BloomAVX2Large3<bloom::Blocked256, 62759>,
            BloomAVX512Large1<bloom::Blocked256, 9856>, BloomAVX512Large2<bloom::Blocked256, 62980>,
            BloomAVX512Large3<bloom::Blocked256, 63135>>;

    using BloomBlocked512TestTypes = ::testing::Types<BloomScalarSmall<bloom::Blocked512, 1>,
            BloomAVXSmall1<bloom::Blocked512, 2>, BloomAVXSmall2<bloom::Blocked512, 0>,
            BloomScalarLarge1<bloom::Blocked512, 6992>, BloomScalarLarge2<bloom::Blocked512, 50513>,
            BloomScalarLarge3<bloom::Blocked512, 50680>, BloomAVX2Large1<bloom::Blocked512, 6860>,
            BloomAVX2Large2<bloom::Blocked512, 50513>, BloomAVX2Large3<bloom::Blocked512, 50440>,
            BloomAVX512Large1<bloom::Blocked512, 6828>, BloomAVX512Large2<bloom::Blocked512, 50611>,
            BloomAVX512Large3<bloom::Blocked512, 50871>>;

    using BloomSectorized128TestTypes = ::testing::Types<BloomScalarSmall<bloom::Sectorized128, 3>,
            BloomAVXSmall1<bloom::Sectorized128, 14>, BloomAVXSmall2<bloom::Sectorized128, 2>,
            BloomScalarLarge1<bloom::Sectorized128, 17673>, BloomScalarLarge2<bloom::Sectorized128, 92143>,
            BloomScalarLarge3<bloom::Sectorized128, 92475>, BloomAVX2Large1<bloom::Sectorized128, 17658>,
            BloomAVX2Large2<bloom::Sectorized128, 92143>, BloomAVX2Large3<bloom::Sectorized128, 92575>,
            BloomAVX512Large1<bloom::Sectorized128, 17999>, BloomAVX512Large2<bloom::Sectorized128, 90079>,
            BloomAVX512Large3<bloom::Sectorized128, 90957>>;

    using BloomSectorized256TestTypes = ::testing::Types<BloomScalarSmall<bloom::Sectorized256, 3>,
            BloomAVXSmall1<bloom::Sectorized256, 3>, BloomAVXSmall2<bloom::Sectorized256, 0>,
            BloomScalarLarge1<bloom::Sectorized256, 10121>, BloomScalarLarge2<bloom::Sectorized256, 66033>,
            BloomScalarLarge3<bloom::Sectorized256, 65896>, BloomAVX2Large1<bloom::Sectorized256, 9977>,
            BloomAVX2Large2<bloom::Sectorized256, 66033>, BloomAVX2Large3<bloom::Sectorized256, 65821>,
            BloomAVX512Large1<bloom::Sectorized256, 10068>, BloomAVX512Large2<bloom::Sectorized256, 63698>,
            BloomAVX512Large3<bloom::Sectorized256, 64607>>;

    using BloomSectorized512TestTypes = ::testing::Types<BloomScalarSmall<bloom::Sectorized512, 1>,
            BloomAVXSmall1<bloom::Sectorized512, 0>, BloomAVXSmall2<bloom::Sectorized512, 0>,
            BloomScalarLarge1<bloom::Sectorized512, 7186>, BloomAVX2Large1<bloom::Sectorized512, 7160>,
            BloomAVX512Large2<bloom::Sectorized512, 51566>, BloomAVX512Large3<bloom::Sectorized512, 51797>>;

    using BloomGrouped2TestTypes = ::testing::Types<BloomScalarSmall<bloom::Grouped2, 4>,
            BloomAVXSmall1<bloom::Grouped2, 7>, BloomAVXSmall2<bloom::Grouped2, 0>,
            BloomScalarLarge1<bloom::Grouped2, 10619>, BloomScalarLarge2<bloom::Grouped2, 86941>,
            BloomScalarLarge3<bloom::Grouped2, 88052>, BloomAVX2Large1<bloom::Grouped2, 10917>,
            BloomAVX2Large2<bloom::Grouped2, 86941>, BloomAVX2Large3<bloom::Grouped2, 86996>,
            BloomAVX512Large1<bloom::Grouped2, 17480>, BloomAVX512Large2<bloom::Grouped2, 65300>,
            BloomAVX512Large3<bloom::Grouped2, 66280>>;

    using BloomGrouped4TestTypes = ::testing::Types<BloomScalarSmall<bloom::Grouped4, 2>,
            BloomAVXSmall1<bloom::Grouped4, 0>, BloomAVXSmall2<bloom::Grouped4, 0>,
            BloomScalarLarge1<bloom::Grouped4, 7232>, BloomScalarLarge2<bloom::Grouped4, 55140>,
            BloomScalarLarge3<bloom::Grouped4, 55307>, BloomAVX2Large1<bloom::Grouped4, 7295>,
            BloomAVX2Large2<bloom::Grouped4, 55140>, BloomAVX2Large3<bloom::Grouped4, 55479>,
            BloomAVX512Large1<bloom::Grouped4, 8111>, BloomAVX512Large2<bloom::Grouped4, 52575>,
            BloomAVX512Large3<bloom::Grouped4, 52999>>;

    using BloomGrouped8TestTypes = ::testing::Types<BloomScalarSmall<bloom::Grouped8, 2>,
            BloomAVXSmall1<bloom::Grouped8, 0>, BloomAVXSmall2<bloom::Grouped8, 0>,
            BloomScalarLarge1<bloom::Grouped8, 7186>, BloomScalarLarge2<bloom::Grouped8, 51602>,
            BloomScalarLarge3<bloom::Grouped8, 51994>, BloomAVX2Large1<bloom::Grouped8, 7160>,
            BloomAVX2Large2<bloom::Grouped8, 51602>, BloomAVX2Large3<bloom::Grouped8, 51432>,
            BloomAVX512Large1<bloom::Grouped8, 7035>, BloomAVX512Large2<bloom::Grouped8, 51566>,
            BloomAVX512Large3<bloom::Grouped8, 51797>>;

    using BloomSectorized128VerticalTestTypes = ::testing::Types<BloomAVXSmall1<bloom::Sectorized128Vertical, 10>,
            BloomAVXSmall2<bloom::Sectorized128Vertical, 3>, BloomAVX2Large1<bloom::Sectorized128Vertical, 17658>,
            BloomAVX2Large2<bloom::Sectorized128Vertical, 92708>, BloomAVX2Large3<bloom::Sectorized128Vertical, 93193>,
            BloomAVX512Large1<bloom::Sectorized128Vertical, 18131>,
            BloomAVX512Large2<bloom::Sectorized128Vertical, 89994>,
            BloomAVX512Large3<bloom::Sectorized128Vertical, 91190>>;

    using BloomSectorized256VerticalTestTypes = ::testing::Types<BloomAVXSmall1<bloom::Sectorized256Vertical, 4>,
            BloomAVXSmall2<bloom::Sectorized256Vertical, 0>, BloomAVX2Large1<bloom::Sectorized256Vertical, 10191>,
            BloomAVX2Large2<bloom::Sectorized256Vertical, 65857>, BloomAVX2Large3<bloom::Sectorized256Vertical, 65701>,
            BloomAVX512Large1<bloom::Sectorized256Vertical, 10355>,
            BloomAVX512Large2<bloom::Sectorized256Vertical, 64146>,
            BloomAVX512Large3<bloom::Sectorized256Vertical, 64781>>;

    using BloomSectorized512VerticalTestTypes = ::testing::Types<BloomAVXSmall2<bloom::Sectorized512Vertical, 0>,
            BloomAVX512Large2<bloom::Sectorized512Vertical, 51605>,
            BloomAVX512Large3<bloom::Sectorized512Vertical, 51536>>;
}

