#pragma once

#include <cstddef>
#include <xor/xor_parameter.hpp>
#include "../filter_test.hpp"

namespace test::nxor {

    namespace parameter = filters::parameter;
    namespace nxor = filters::nxor;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k_s = 8, k_l = 16;

    static constexpr size_t n_partition_s = 2, n_partition_l = 16;

    static constexpr filters::FilterType Xor = filters::FilterType::Xor;

    /*
     * Small Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorScalarSmall = FilterTestConfig<Xor,
            FP, 10, parameter::MagicMurmurScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVXSmall1 = FilterTestConfig<Xor, FP,
            7, parameter::LemireMurmurAVX232Partitioned, n_s, s, n_partition_s, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVXSmall2 = FilterTestConfig<Xor, FP,
            13, parameter::PowerOfTwoMurmurAVX51264Partitioned, n_s, s, n_partition_l, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorScalarLarge1 = FilterTestConfig<Xor,
            FP, 15, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorScalarLarge2 = FilterTestConfig<Xor,
            FP, 8, parameter::MagicMurmurScalar32, n_l, s, 0, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorScalarLarge3 = FilterTestConfig<Xor,
            FP, 6, parameter::LemireMurmurScalar32PartitionedMT, n_l, s, n_partition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVX2Large1 = FilterTestConfig<Xor,
            FP, 16, parameter::PowerOfTwoMurmurAVX264Partitioned, n_l, s, n_partition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVX2Large2 = FilterTestConfig<Xor,
            FP, 5, parameter::MagicMurmurAVX232PartitionedMT, n_l, s, n_partition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVX2Large3 = FilterTestConfig<Xor,
            FP, 4, parameter::LemireMurmurAVX232Partitioned, n_l, s, n_partition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVX512Large1 = FilterTestConfig<Xor,
            FP, 8, parameter::PowerOfTwoMurmurAVX51232PartitionedMT, n_l, s, n_partition_l, 4, 16, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVX512Large2 = FilterTestConfig<Xor,
            FP, 11, parameter::MagicMurmurAVX51264Partitioned, n_l, s, n_partition_l, 0, 0, expected_fp>;
    template<template<size_t> typename FP, size_t s, int64_t expected_fp> using XorAVX512Large3 = FilterTestConfig<Xor,
            FP, 9, parameter::LemireMurmurAVX51264Partitioned, n_l, s, n_partition_l, 0, 0, expected_fp>;


    /*
     * Variant Test Types
     */

    using XorStandardTestTypes = ::testing::Types<XorScalarSmall<nxor::Standard, 125, 8>,
            XorAVXSmall1<nxor::Standard, 125, 67>, XorAVXSmall2<nxor::Standard, 125, 3>,
            XorScalarLarge1<nxor::Standard, 125, 326>, XorScalarLarge2<nxor::Standard, 125, 39041>,
            XorScalarLarge3<nxor::Standard, 125, 156567>, XorAVX2Large1<nxor::Standard, 125, 159>,
            XorAVX2Large2<nxor::Standard, 125, 312591>, XorAVX2Large3<nxor::Standard, 125, 624988>,
            XorAVX512Large1<nxor::Standard, 125, 40397>, XorAVX512Large2<nxor::Standard, 125, 4926>,
            XorAVX512Large3<nxor::Standard, 125, 19320>>;

    using XorFuseTestTypes = ::testing::Types<XorScalarSmall<nxor::Fuse, 300, 7>, XorAVXSmall1<nxor::Fuse, 500, 89>,
            XorAVXSmall2<nxor::Fuse, 500, 0>, XorScalarLarge1<nxor::Fuse, 110, 307>,
            XorScalarLarge2<nxor::Fuse, 113, 39016>, XorScalarLarge3<nxor::Fuse, 117, 156233>,
            XorAVX2Large1<nxor::Fuse, 110, 148>, XorAVX2Large2<nxor::Fuse, 130, 313145>,
            XorAVX2Large3<nxor::Fuse, 117, 624459>, XorAVX512Large1<nxor::Fuse, 110, 39440>,
            XorAVX512Large2<nxor::Fuse, 117, 4930>, XorAVX512Large3<nxor::Fuse, 117, 19613>>;
}
