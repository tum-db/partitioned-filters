#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::morton_ota {

    namespace parameter = filters::parameter;
    namespace cuckoo = filters::cuckoo;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t n_partitions_s = 2, n_partitition_l = 16;

    static constexpr filters::FilterType Cuckoo = filters::FilterType::Cuckoo;

    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarSmall1 = FilterTestConfig<
            Cuckoo, FP, 10, parameter::MagicMurmurScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarSmall2 = FilterTestConfig<
            Cuckoo, FP, 4, parameter::VacuumMurmurScalar32Partitioned, n_s, s, n_partitions_s, 2, 2, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarSmall3 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::MagicMurmurScalar64, n_s, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVXSmall1 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::LemireMurmurAVX232, n_s, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVXSmall2 = FilterTestConfig<
            Cuckoo, FP, 13, parameter::PowerOfTwoMurmurAVX51264Partitioned, n_s, s, n_partitition_l, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarLarge1 = FilterTestConfig<
            Cuckoo, FP, 15, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarLarge2 = FilterTestConfig<
            Cuckoo, FP, 4, parameter::MagicMurmurScalar32Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarLarge3 = FilterTestConfig<
            Cuckoo, FP, 6, parameter::LemireMurmurScalar32MT, n_l, s, 0, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarLarge4 = FilterTestConfig<
            Cuckoo, FP, 13, parameter::VacuumMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonScalarLarge5 = FilterTestConfig<
            Cuckoo, FP, 16, parameter::LemireMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX2Large1 = FilterTestConfig<
            Cuckoo, FP, 16, parameter::PowerOfTwoMurmurAVX264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX2Large2 = FilterTestConfig<
            Cuckoo, FP, 5, parameter::MagicMurmurAVX232PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX2Large3 = FilterTestConfig<
            Cuckoo, FP, 2, parameter::LemireMurmurAVX232Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX2Large4 = FilterTestConfig<
            Cuckoo, FP, 9, parameter::VacuumMurmurAVX264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX512Large1 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::PowerOfTwoMurmurAVX51232PartitionedMT, n_l, s, n_partitition_l, 4, 16,
            expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX512Large2 = FilterTestConfig<
            Cuckoo, FP, 11, parameter::MagicMurmurAVX51264, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX512Large3 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::LemireMurmurAVX51264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using MortonAVX512Large4 = FilterTestConfig<
            Cuckoo, FP, 12, parameter::VacuumMurmurAVX51264PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;


    /*
     * Variant Test Types
     */

    using CuckooMortonOTA2_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton2_30_5, 135, 11>,
            MortonScalarSmall2<cuckoo::Morton2_30_10, 188, 369>, MortonScalarSmall3<cuckoo::Morton2_30_15, 145, 26>,
            MortonAVXSmall1<cuckoo::Morton2_30_20, 146, 38>, MortonAVXSmall2<cuckoo::Morton2_30_25, 129, 0>,
            MortonScalarLarge1<cuckoo::Morton2_30_30, 126, 140>, MortonScalarLarge2<cuckoo::Morton2_30_35, 198, 488315>,
            MortonScalarLarge3<cuckoo::Morton2_30_5, 159, 148827>, MortonScalarLarge4<cuckoo::Morton2_30_10, 127, 1047>,
            MortonScalarLarge5<cuckoo::Morton2_30_15, 124, 399>, MortonAVX2Large1<cuckoo::Morton2_30_20, 123, 89>,
            MortonAVX2Large2<cuckoo::Morton2_30_25, 177, 254212>, MortonAVX2Large3<cuckoo::Morton2_30_30, 400, 1093620>,
            MortonAVX2Large4<cuckoo::Morton2_30_35, 148, 12524>, MortonAVX512Large1<cuckoo::Morton2_30_5, 143, 23325>,
            MortonAVX512Large2<cuckoo::Morton2_30_10, 133, 4659>, MortonAVX512Large3<cuckoo::Morton2_30_15, 146, 34682>,
            MortonAVX512Large4<cuckoo::Morton2_30_20, 137, 1630>>;

    using CuckooMortonOTA3_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton3_30_10, 124, 9>,
            MortonScalarSmall2<cuckoo::Morton3_30_15, 160, 308>, MortonScalarSmall3<cuckoo::Morton3_30_20, 131, 48>,
            MortonAVXSmall1<cuckoo::Morton3_30_25, 132, 38>, MortonAVXSmall2<cuckoo::Morton3_30_30, 120, 0>,
            MortonScalarLarge1<cuckoo::Morton3_30_35, 117, 130>, MortonScalarLarge2<cuckoo::Morton3_30_5, 157, 945727>,
            MortonScalarLarge3<cuckoo::Morton3_30_10, 141, 203177>,
            MortonScalarLarge4<cuckoo::Morton3_30_15, 119, 1275>, MortonScalarLarge5<cuckoo::Morton3_30_20, 119, 218>,
            MortonAVX2Large1<cuckoo::Morton3_30_25, 116, 84>, MortonAVX2Large2<cuckoo::Morton3_30_30, 152, 365478>,
            MortonAVX2Large3<cuckoo::Morton3_30_35, 250, 2004865>, MortonAVX2Large4<cuckoo::Morton3_30_5, 126, 21856>,
            MortonAVX512Large1<cuckoo::Morton3_30_10, 130, 18564>, MortonAVX512Large2<cuckoo::Morton3_30_15, 123, 6425>,
            MortonAVX512Large3<cuckoo::Morton3_30_20, 131, 51735>,
            MortonAVX512Large4<cuckoo::Morton3_30_25, 121, 2539>>;

    using CuckooMortonOTA7_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton7_30_15, 115, 32>,
            MortonScalarSmall2<cuckoo::Morton7_30_20, 139, 1253>, MortonScalarSmall3<cuckoo::Morton7_30_25, 120, 99>,
            MortonAVXSmall1<cuckoo::Morton7_30_30, 120, 96>, MortonAVXSmall2<cuckoo::Morton7_30_35, 113, 0>,
            MortonScalarLarge1<cuckoo::Morton7_30_5, 110, 640>, MortonScalarLarge2<cuckoo::Morton7_30_10, 137, 1986818>,
            MortonScalarLarge3<cuckoo::Morton7_30_15, 126, 491420>,
            MortonScalarLarge4<cuckoo::Morton7_30_20, 113, 3024>, MortonScalarLarge5<cuckoo::Morton7_30_25, 111, 539>,
            MortonAVX2Large1<cuckoo::Morton7_30_30, 110, 339>, MortonAVX2Large2<cuckoo::Morton7_30_35, 132, 900700>,
            MortonAVX2Large3<cuckoo::Morton7_30_5, 173, 5876997>, MortonAVX2Large4<cuckoo::Morton7_30_10, 117, 59082>,
            MortonAVX512Large1<cuckoo::Morton7_30_15, 119, 83281>,
            MortonAVX512Large2<cuckoo::Morton7_30_20, 114, 17730>,
            MortonAVX512Large3<cuckoo::Morton7_30_25, 121, 117408>,
            MortonAVX512Large4<cuckoo::Morton7_30_30, 114, 7846>>;

    using CuckooMortonOTA15_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton15_30_20, 110, 65>,
            MortonScalarSmall2<cuckoo::Morton15_30_25, 124, 2107>, MortonScalarSmall3<cuckoo::Morton15_30_30, 112, 264>,
            MortonAVXSmall1<cuckoo::Morton15_30_35, 113, 262>, MortonAVXSmall2<cuckoo::Morton15_30_5, 107, 4>,
            MortonScalarLarge1<cuckoo::Morton15_30_10, 107, 1283>,
            MortonScalarLarge2<cuckoo::Morton15_30_15, 124, 3668004>,
            MortonScalarLarge3<cuckoo::Morton15_30_20, 116, 1122322>,
            MortonScalarLarge4<cuckoo::Morton15_30_25, 108, 7112>,
            MortonScalarLarge5<cuckoo::Morton15_30_30, 107, 1236>, MortonAVX2Large1<cuckoo::Morton15_30_35, 107, 621>,
            MortonAVX2Large2<cuckoo::Morton15_30_5, 118, 2348978>,
            MortonAVX2Large3<cuckoo::Morton15_30_10, 146, 8486458>,
            MortonAVX2Large4<cuckoo::Morton15_30_15, 111, 103931>,
            MortonAVX512Large1<cuckoo::Morton15_30_20, 112, 151112>,
            MortonAVX512Large2<cuckoo::Morton15_30_25, 109, 38995>,
            MortonAVX512Large3<cuckoo::Morton15_30_30, 113, 274481>,
            MortonAVX512Large4<cuckoo::Morton15_30_35, 109, 15431>>;

    using CuckooMortonOTA3_25TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton3_25_25, 130, 12>,
            MortonScalarSmall2<cuckoo::Morton3_25_30, 177, 304>, MortonScalarSmall3<cuckoo::Morton3_25_35, 140, 42>,
            MortonAVXSmall1<cuckoo::Morton3_25_5, 135, 29>, MortonAVXSmall2<cuckoo::Morton3_25_10, 122, 0>,
            MortonScalarLarge1<cuckoo::Morton3_25_15, 120, 131>, MortonScalarLarge2<cuckoo::Morton3_25_20, 174, 615948>,
            MortonScalarLarge3<cuckoo::Morton3_25_25, 150, 160143>,
            MortonScalarLarge4<cuckoo::Morton3_25_30, 124, 990>, MortonScalarLarge5<cuckoo::Morton3_25_35, 124, 167>,
            MortonAVX2Large1<cuckoo::Morton3_25_5, 118, 85>, MortonAVX2Large2<cuckoo::Morton3_25_10, 156, 359049>,
            MortonAVX2Large3<cuckoo::Morton3_25_15, 250, 2047163>, MortonAVX2Large4<cuckoo::Morton3_25_20, 133, 15973>,
            MortonAVX512Large1<cuckoo::Morton3_25_25, 138, 18481>, MortonAVX512Large2<cuckoo::Morton3_25_30, 130, 4893>,
            MortonAVX512Large3<cuckoo::Morton3_25_35, 140, 38464>, MortonAVX512Large4<cuckoo::Morton3_25_5, 123, 2124>>;
}
