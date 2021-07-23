#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::cuckoo {

    namespace parameter = filters::parameter;
    namespace cuckoo = filters::cuckoo;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t n_partitions_s = 2, n_partitition_l = 16;

    static constexpr filters::FilterType Cuckoo = filters::FilterType::Cuckoo;

    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarSmall1 = FilterTestConfig<
            Cuckoo, FP, 10, parameter::MagicMurmurScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarSmall2 = FilterTestConfig<
            Cuckoo, FP, 4, parameter::VacuumMurmurScalar32PartitionedMT, n_s, s, n_partitions_s, 2, 2, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarSmall3 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::MagicMurmurScalar64, n_s, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVXSmall1 = FilterTestConfig<
            Cuckoo, FP, 7, parameter::LemireMurmurAVX232Partitioned, n_s, s, n_partitions_s, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVXSmall2 = FilterTestConfig<
            Cuckoo, FP, 13, parameter::PowerOfTwoMurmurAVX51264Partitioned, n_s, s, n_partitition_l, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarLarge1 = FilterTestConfig<
            Cuckoo, FP, 15, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarLarge2 = FilterTestConfig<
            Cuckoo, FP, 4, parameter::MagicMurmurScalar32Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarLarge3 = FilterTestConfig<
            Cuckoo, FP, 6, parameter::LemireMurmurScalar32MT, n_l, s, 0, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooScalarLarge4 = FilterTestConfig<
            Cuckoo, FP, 13, parameter::VacuumMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX2Large1 = FilterTestConfig<
            Cuckoo, FP, 16, parameter::PowerOfTwoMurmurAVX264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX2Large2 = FilterTestConfig<
            Cuckoo, FP, 5, parameter::MagicMurmurAVX232PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX2Large3 = FilterTestConfig<
            Cuckoo, FP, 2, parameter::LemireMurmurAVX232Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX2Large4 = FilterTestConfig<
            Cuckoo, FP, 9, parameter::VacuumMurmurAVX264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX512Large1 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::PowerOfTwoMurmurAVX51232PartitionedMT, n_l, s, n_partitition_l, 4, 16,
            expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX512Large2 = FilterTestConfig<
            Cuckoo, FP, 11, parameter::MagicMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX512Large3 = FilterTestConfig<
            Cuckoo, FP, 8, parameter::LemireMurmurAVX51264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using CuckooAVX512Large4 = FilterTestConfig<
            Cuckoo, FP, 12, parameter::VacuumMurmurAVX51264PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;


    /*
     * Variant Test Types
     */

    using CuckooStandard2TestTypes = ::testing::Types<CuckooScalarSmall1<cuckoo::Standard2, 115, 40>,
            CuckooScalarSmall2<cuckoo::Standard2, 105, 1223>, CuckooScalarSmall3<cuckoo::Standard2, 115, 131>,
            CuckooAVXSmall1<cuckoo::Standard2, 115, 300>, CuckooAVXSmall2<cuckoo::Standard2, 120, 1>,
            CuckooScalarLarge1<cuckoo::Standard2, 105, 540>, CuckooScalarLarge3<cuckoo::Standard2, 130, 481368>,
            CuckooScalarLarge4<cuckoo::Standard2, 105, 4047>, CuckooAVX2Large1<cuckoo::Standard2, 106, 305>,
            CuckooAVX2Large2<cuckoo::Standard2, 180, 710340>, CuckooAVX2Large4<cuckoo::Standard2, 110, 65935>,
            CuckooAVX512Large1<cuckoo::Standard2, 106, 74826>, CuckooAVX512Large2<cuckoo::Standard2, 116, 16765>,
            CuckooAVX512Large3<cuckoo::Standard2, 114, 137032>, CuckooAVX512Large4<cuckoo::Standard2, 110, 8465>>;

    using CuckooStandard3TestTypes = ::testing::Types<CuckooScalarSmall1<cuckoo::Standard3, 110, 63>,
            CuckooScalarSmall2<cuckoo::Standard3, 100, 2401>, CuckooScalarSmall3<cuckoo::Standard3, 115, 187>,
            CuckooAVXSmall1<cuckoo::Standard3, 115, 406>, CuckooAVXSmall2<cuckoo::Standard3, 105, 4>,
            CuckooScalarLarge1<cuckoo::Standard3, 105, 1137>, CuckooScalarLarge3<cuckoo::Standard3, 120, 771064>,
            CuckooScalarLarge4<cuckoo::Standard3, 110, 6053>, CuckooAVX2Large1<cuckoo::Standard3, 105, 610>,
            CuckooAVX2Large2<cuckoo::Standard3, 130, 1414735>, CuckooAVX2Large4<cuckoo::Standard3, 110, 98703>,
            CuckooAVX512Large1<cuckoo::Standard3, 105, 148974>, CuckooAVX512Large2<cuckoo::Standard3, 110, 26551>,
            CuckooAVX512Large3<cuckoo::Standard3, 106, 220518>, CuckooAVX512Large4<cuckoo::Standard3, 110, 12541>>;

    using CuckooStandard4TestTypes = ::testing::Types<CuckooAVXSmall1<cuckoo::Standard4, 103, 627>,
            CuckooScalarSmall2<cuckoo::Standard4, 105, 2401>, CuckooScalarSmall3<cuckoo::Standard4, 105, 306>,
            CuckooAVXSmall2<cuckoo::Standard4, 105, 5>, CuckooScalarLarge1<cuckoo::Standard4, 100, 2266>,
            CuckooScalarLarge2<cuckoo::Standard4, 130, 3462344>, CuckooScalarLarge3<cuckoo::Standard4, 110, 1099909>,
            CuckooScalarLarge4<cuckoo::Standard4, 100, 9206>, CuckooAVX2Large1<cuckoo::Standard4, 100, 1190>,
            CuckooAVX2Large2<cuckoo::Standard4, 108, 2168348>, CuckooAVX2Large4<cuckoo::Standard4, 110, 132038>,
            CuckooAVX512Large1<cuckoo::Standard4, 100, 295389>, CuckooAVX512Large2<cuckoo::Standard4, 104, 37032>,
            CuckooAVX512Large3<cuckoo::Standard4, 103, 300062>, CuckooAVX512Large4<cuckoo::Standard4, 110, 16763>>;

    using CuckooStandard5TestTypes = ::testing::Types<CuckooScalarSmall2<cuckoo::Standard5, 100, 4153>,
            CuckooScalarSmall3<cuckoo::Standard5, 105, 368>, CuckooScalarLarge2<cuckoo::Standard5, 120, 4357566>,
            CuckooScalarLarge3<cuckoo::Standard5, 105, 1417248>, CuckooAVX2Large2<cuckoo::Standard5, 115, 2490570>,
            CuckooAVX2Large4<cuckoo::Standard5, 100, 182928>, CuckooAVX512Large2<cuckoo::Standard5, 105, 46443>,
            CuckooAVX512Large3<cuckoo::Standard5, 103, 374700>, CuckooAVX512Large4<cuckoo::Standard5, 100, 22959>>;

    using CuckooStandard6TestTypes = ::testing::Types<CuckooScalarSmall2<cuckoo::Standard6, 100, 4153>,
            CuckooScalarSmall3<cuckoo::Standard6, 105, 459>, CuckooScalarLarge2<cuckoo::Standard6, 115, 5099978>,
            CuckooAVX2Large2<cuckoo::Standard6, 110, 3016308>, CuckooAVX2Large4<cuckoo::Standard6, 100, 215609>,
            CuckooAVX512Large3<cuckoo::Standard6, 102, 452702>>;

    using CuckooStandard7TestTypes = ::testing::Types<CuckooScalarSmall2<cuckoo::Standard7, 100, 4153>,
            CuckooScalarSmall3<cuckoo::Standard7, 105, 516>, CuckooScalarLarge2<cuckoo::Standard7, 120, 5484553>,
            CuckooAVX2Large4<cuckoo::Standard7, 105, 235102>, CuckooAVX512Large3<cuckoo::Standard7, 102, 525674>>;

    using CuckooStandard8TestTypes = ::testing::Types<CuckooScalarSmall2<cuckoo::Standard8, 100, 6551>,
            CuckooScalarSmall3<cuckoo::Standard8, 105, 603>, CuckooScalarLarge2<cuckoo::Standard8, 110, 6264863>,
            CuckooAVX2Large3<cuckoo::Standard8, 190, 9310645>, CuckooAVX512Large3<cuckoo::Standard8, 101, 605467>>;
}


