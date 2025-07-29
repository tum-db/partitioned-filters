#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::umbra
{
    namespace umbra = filters::umbra;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 400;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k = 4;

    static constexpr filters::FilterType UmbraBloom = filters::FilterType::UmbraBloom;


    /*
     * Small Test Types
     */

    template <int64_t expected_fp>
    using UmbraBloomSmall1 = FilterTestConfig<UmbraBloom, umbra::Standard, k, parameter::PowerOfTwoMulScalar64, n_s, s, 0, 0, 0, expected_fp>;
    template <int64_t expected_fp>
    using UmbraBloomSmall2 = FilterTestConfig<UmbraBloom, umbra::Standard, k, parameter::PowerOfTwoMulScalar64MT, n_s, s, 0, 2, 2, expected_fp>;
    template <int64_t expected_fp>
    using UmbraBloomSmall3 = FilterTestConfig<UmbraBloom, umbra::Standard, k, parameter::PowerOfTwoMurmurScalar64, n_s, s, 0, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template <int64_t expected_fp>
    using UmbraBloomLarge1 = FilterTestConfig<UmbraBloom, umbra::Standard, k, parameter::PowerOfTwoMulScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template <int64_t expected_fp>
    using UmbraBloomLarge2 = FilterTestConfig<UmbraBloom, umbra::Standard, k, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template <int64_t expected_fp>
    using UmbraBloomLarge3 = FilterTestConfig<UmbraBloom, umbra::Standard, k, parameter::PowerOfTwoMurmurScalar64MT, n_l, s, 0, 4, 16, expected_fp>;


    /*
     * Test Types
     */

    using UmbraBloomTestTypes = ::testing::Types<
        UmbraBloomSmall1<132>, UmbraBloomSmall2<132>, UmbraBloomSmall3<130>,
        UmbraBloomLarge1<101597>, UmbraBloomLarge2<125215>, UmbraBloomLarge3<125215>>;
}
