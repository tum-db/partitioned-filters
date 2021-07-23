#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::morton {

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

    using CuckooMorton2_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton2_30_0, 134, 17>,
            MortonScalarSmall2<cuckoo::Morton2_30_0, 120, 600>, MortonScalarSmall3<cuckoo::Morton2_30_0, 142, 49>,
            MortonAVXSmall1<cuckoo::Morton2_30_0, 142, 42>, MortonAVXSmall2<cuckoo::Morton2_30_0, 100, 1>,
            MortonScalarLarge1<cuckoo::Morton2_30_0, 123, 262>, MortonScalarLarge2<cuckoo::Morton2_30_0, 184, 721935>,
            MortonScalarLarge3<cuckoo::Morton2_30_0, 156, 184349>, MortonScalarLarge4<cuckoo::Morton2_30_0, 126, 1418>,
            MortonScalarLarge5<cuckoo::Morton2_30_0, 124, 531>, MortonAVX2Large1<cuckoo::Morton2_30_0, 124, 152>,
            MortonAVX2Large2<cuckoo::Morton2_30_0, 168, 365282>, MortonAVX2Large3<cuckoo::Morton2_30_0, 400, 1811828>,
            MortonAVX2Large4<cuckoo::Morton2_30_0, 142, 21297>, MortonAVX512Large1<cuckoo::Morton2_30_0, 142, 36983>,
            MortonAVX512Large2<cuckoo::Morton2_30_0, 133, 5804>, MortonAVX512Large3<cuckoo::Morton2_30_0, 143, 46606>,
            MortonAVX512Large4<cuckoo::Morton2_30_0, 128, 2932>>;

    using CuckooMorton3_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton3_30_0, 123, 7>,
            MortonScalarSmall2<cuckoo::Morton3_30_0, 156, 585>, MortonScalarSmall3<cuckoo::Morton3_30_0, 128, 76>,
            MortonAVXSmall1<cuckoo::Morton3_30_0, 129, 62>, MortonAVXSmall2<cuckoo::Morton3_30_0, 118, 1>,
            MortonScalarLarge1<cuckoo::Morton3_30_0, 115, 262>, MortonScalarLarge2<cuckoo::Morton3_30_0, 157, 1058123>,
            MortonScalarLarge3<cuckoo::Morton3_30_0, 138, 273928>, MortonScalarLarge4<cuckoo::Morton3_30_0, 117, 2074>,
            MortonScalarLarge5<cuckoo::Morton3_30_0, 115, 286>, MortonAVX2Large1<cuckoo::Morton3_30_0, 114, 152>,
            MortonAVX2Large2<cuckoo::Morton3_30_0, 145, 549414>, MortonAVX2Large3<cuckoo::Morton3_30_0, 250, 3177693>,
            MortonAVX2Large4<cuckoo::Morton3_30_0, 125, 32920>, MortonAVX512Large1<cuckoo::Morton3_30_0, 128, 36983>,
            MortonAVX512Large2<cuckoo::Morton3_30_0, 123, 8537>, MortonAVX512Large3<cuckoo::Morton3_30_0, 128, 70976>,
            MortonAVX512Large4<cuckoo::Morton3_30_0, 119, 4110>>;

    using CuckooMorton7_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton7_30_0, 115, 39>,
            MortonScalarSmall2<cuckoo::Morton7_30_0, 136, 2079>, MortonScalarSmall3<cuckoo::Morton7_30_0, 118, 153>,
            MortonAVXSmall1<cuckoo::Morton7_30_0, 118, 164>, MortonAVXSmall2<cuckoo::Morton7_30_0, 111, 3>,
            MortonScalarLarge1<cuckoo::Morton7_30_0, 110, 1137>, MortonScalarLarge2<cuckoo::Morton7_30_0, 136, 2343632>,
            MortonScalarLarge3<cuckoo::Morton7_30_0, 124, 646333>, MortonScalarLarge4<cuckoo::Morton7_30_0, 113, 5018>,
            MortonScalarLarge5<cuckoo::Morton7_30_0, 111, 709>, MortonAVX2Large1<cuckoo::Morton7_30_0, 109, 596>,
            MortonAVX2Large2<cuckoo::Morton7_30_0, 129, 1253377>, MortonAVX2Large3<cuckoo::Morton7_30_0, 175, 6430756>,
            MortonAVX2Large4<cuckoo::Morton7_30_0, 116, 78588>, MortonAVX512Large1<cuckoo::Morton7_30_0, 118, 147921>,
            MortonAVX512Large2<cuckoo::Morton7_30_0, 114, 21212>, MortonAVX512Large3<cuckoo::Morton7_30_0, 119, 166920>,
            MortonAVX512Large4<cuckoo::Morton7_30_0, 112, 9961>>;

    using CuckooMorton15_30TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton15_30_0, 109, 102>,
            MortonScalarSmall2<cuckoo::Morton15_30_0, 123, 3785>, MortonScalarSmall3<cuckoo::Morton15_30_0, 112, 365>,
            MortonAVXSmall1<cuckoo::Morton15_30_0, 112, 380>, MortonAVXSmall2<cuckoo::Morton15_30_0, 107, 10>,
            MortonScalarLarge1<cuckoo::Morton15_30_0, 106, 2266>,
            MortonScalarLarge2<cuckoo::Morton15_30_0, 123, 4317410>,
            MortonScalarLarge3<cuckoo::Morton15_30_0, 116, 1334072>,
            MortonScalarLarge4<cuckoo::Morton15_30_0, 107, 10579>, MortonScalarLarge5<cuckoo::Morton15_30_0, 107, 1581>,
            MortonAVX2Large1<cuckoo::Morton15_30_0, 106, 1204>, MortonAVX2Large2<cuckoo::Morton15_30_0, 118, 2479556>,
            MortonAVX2Large3<cuckoo::Morton15_30_0, 145, 8956006>, MortonAVX2Large4<cuckoo::Morton15_30_0, 111, 168386>,
            MortonAVX512Large1<cuckoo::Morton15_30_0, 112, 293467>,
            MortonAVX512Large2<cuckoo::Morton15_30_0, 109, 46002>,
            MortonAVX512Large3<cuckoo::Morton15_30_0, 112, 365497>,
            MortonAVX512Large4<cuckoo::Morton15_30_0, 108, 23157>>;

    using CuckooMorton3_25TestTypes = ::testing::Types<MortonScalarSmall1<cuckoo::Morton3_25_0, 127, 10>,
            MortonScalarSmall2<cuckoo::Morton3_25_0, 166, 585>, MortonScalarSmall3<cuckoo::Morton3_25_0, 134, 67>,
            MortonAVXSmall1<cuckoo::Morton3_25_0, 134, 54>, MortonAVXSmall2<cuckoo::Morton3_25_0, 121, 1>,
            MortonScalarLarge1<cuckoo::Morton3_25_0, 119, 262>, MortonScalarLarge2<cuckoo::Morton3_25_0, 167, 896867>,
            MortonScalarLarge3<cuckoo::Morton3_25_0, 145, 231322>, MortonScalarLarge4<cuckoo::Morton3_25_0, 121, 1694>,
            MortonScalarLarge5<cuckoo::Morton3_25_0, 119, 244>, MortonAVX2Large1<cuckoo::Morton3_25_0, 117, 152>,
            MortonAVX2Large2<cuckoo::Morton3_25_0, 156, 450553>, MortonAVX2Large3<cuckoo::Morton3_25_0, 250, 2959630>,
            MortonAVX2Large4<cuckoo::Morton3_25_0, 130, 26812>, MortonAVX512Large1<cuckoo::Morton3_25_0, 134, 36983>,
            MortonAVX512Large2<cuckoo::Morton3_25_0, 126, 7278>, MortonAVX512Large3<cuckoo::Morton3_25_0, 134, 58174>,
            MortonAVX512Large4<cuckoo::Morton3_25_0, 123, 3368>>;
}
