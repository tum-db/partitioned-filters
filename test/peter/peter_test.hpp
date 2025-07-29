#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::peter
{
    namespace peter = filters::peter;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 144;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k = 7;

    static constexpr filters::FilterType PeterBloom = filters::FilterType::PeterBloom;


    /*
     * Small Test Types
     */

    template <int64_t expected_fp>
    using PeterBloomSmall1 = FilterTestConfig<PeterBloom, peter::Standard, k, parameter::PowerOfTwoMulScalar64, n_s, s, 0, 0, 0, expected_fp>;
    template <int64_t expected_fp>
    using PeterBloomSmall2 = FilterTestConfig<PeterBloom, peter::Standard, k, parameter::PowerOfTwoMulAVX51264MT, n_s, s, 0, 2, 2, expected_fp>;
    template <int64_t expected_fp>
    using PeterBloomSmall3 = FilterTestConfig<PeterBloom, peter::Standard, k, parameter::PowerOfTwoMurmurAVX51264, n_s, s, 0, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template <int64_t expected_fp>
    using PeterBloomLarge1 = FilterTestConfig<PeterBloom, peter::Standard, k, parameter::PowerOfTwoMulAVX51264, n_l, s, 0, 0, 0, expected_fp>;
    template <int64_t expected_fp>
    using PeterBloomLarge2 = FilterTestConfig<PeterBloom, peter::Standard, k, parameter::PowerOfTwoMurmurScalar64, n_l, s, 0, 0, 0, expected_fp>;
    template <int64_t expected_fp>
    using PeterBloomLarge3 = FilterTestConfig<PeterBloom, peter::Standard, k, parameter::PowerOfTwoMurmurAVX51264MT, n_l, s, 0, 4, 16, expected_fp>;


    /*
     * Test Types
     */

    using PeterBloomTestTypes = ::testing::Types<
        PeterBloomSmall1<15>, PeterBloomSmall2<15>, PeterBloomSmall3<23>,
        PeterBloomLarge1<15957>, PeterBloomLarge2<17683>, PeterBloomLarge3<17683>>;
}
