#pragma once

#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include "../filter_test.hpp"

namespace test::maskbloom {

    namespace maskbloom = filters::maskbloom;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 400;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k_s = 4, k_l = 4;

    static constexpr size_t n_partitions_s = 2, n_partitition_l = 16;

    static constexpr filters::FilterType MaskBloom = filters::FilterType::MaskBloom;


    /*
     * Small Test Types
     */

    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomScalarSmall = FilterTestConfig<MaskBloom, FP, k_l, parameter::MagicMurmurScalar32MT, n_s, s, 0, 2, 2, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVXSmall1 = FilterTestConfig<MaskBloom, FP, k_l, parameter::LemireMurmurAVX232Partitioned, n_s, s, n_partitions_s, 0, 0, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVXSmall2 = FilterTestConfig<MaskBloom, FP, k_l, parameter::PowerOfTwoMurmurAVX51264Partitioned, n_s, s, n_partitition_l, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomScalarLarge1 = FilterTestConfig<MaskBloom, FP, k_s, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomScalarLarge2 = FilterTestConfig<MaskBloom, FP, k_s, parameter::MagicMurmurScalar32Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomScalarLarge3 = FilterTestConfig<MaskBloom, FP, k_s, parameter::LemireMurmurScalar32MT, n_l, s, 0, 4, 16, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVX2Large1 = FilterTestConfig<MaskBloom, FP, k_s, parameter::PowerOfTwoMurmurAVX264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVX2Large2 = FilterTestConfig<MaskBloom, FP, k_s, parameter::MagicMurmurAVX232PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVX2Large3 = FilterTestConfig<MaskBloom, FP, k_s, parameter::LemireMurmurAVX232Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVX512Large1 = FilterTestConfig<MaskBloom, FP, k_s, parameter::PowerOfTwoMurmurAVX51232PartitionedMT, n_l, s, n_partitition_l, 4, 16, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVX512Large2 = FilterTestConfig<MaskBloom, FP, k_s, parameter::MagicMurmurAVX51264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;
    template <template<size_t> typename FP, int64_t expected_fp>
    using MaskBloomAVX512Large3 = FilterTestConfig<MaskBloom, FP, k_s, parameter::LemireMurmurAVX51264Partitioned, n_l, s, n_partitition_l, 0, 0, expected_fp>;


    /*
     * Variant Test Types
     */

    using MaskBloom4TestTypes = ::testing::Types<
        MaskBloomScalarSmall<maskbloom::MaskBloom4, 478>, MaskBloomAVXSmall1<maskbloom::MaskBloom4, 9822>, MaskBloomAVXSmall2<maskbloom::MaskBloom4, 9846>,
        MaskBloomScalarLarge1<maskbloom::MaskBloom4, 425643>, MaskBloomScalarLarge2<maskbloom::MaskBloom4, 462218>, MaskBloomScalarLarge3<maskbloom::MaskBloom4, 464629>,
        MaskBloomAVX2Large1<maskbloom::MaskBloom4, 9843913>, MaskBloomAVX2Large2<maskbloom::MaskBloom4, 9843601>, MaskBloomAVX2Large3<maskbloom::MaskBloom4, 9843596>,
        MaskBloomAVX512Large1<maskbloom::MaskBloom4, 9843595>, MaskBloomAVX512Large2<maskbloom::MaskBloom4, 9843915>, MaskBloomAVX512Large3<maskbloom::MaskBloom4, 9843915>>;


}
