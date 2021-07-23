#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::impala {

    namespace impala = filters::impala;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 144;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k = 8;

    static constexpr filters::FilterType ImpalaBloom = filters::FilterType::ImpalaBloom;


    /*
     * Small Test Types
     */

    template<int64_t expected_fp> using ImpalaBloomSmall1 = FilterTestConfig<ImpalaBloom, impala::Standard, k,
            parameter::MagicMurmurAVX232, n_s, s, 0, 0, 0, expected_fp>;
    template<int64_t expected_fp> using ImpalaBloomSmall2 = FilterTestConfig<ImpalaBloom, impala::Standard, k,
            parameter::LemireMurmurAVX232MT, n_s, s, 0, 2, 2, expected_fp>;
    template<int64_t expected_fp> using ImpalaBloomSmall3 = FilterTestConfig<ImpalaBloom, impala::Standard, k,
            parameter::PowerOfTwoMurmurAVX264, n_s, s, 0, 0, 0, expected_fp>;

    /*
     * Large Test Types
     */

    template<int64_t expected_fp> using ImpalaBloomLarge1 = FilterTestConfig<ImpalaBloom, impala::Standard, k,
            parameter::MagicMurmurAVX264, n_l, s, 0, 0, 0, expected_fp>;
    template<int64_t expected_fp> using ImpalaBloomLarge2 = FilterTestConfig<ImpalaBloom, impala::Standard, k,
            parameter::LemireMurmurAVX264, n_l, s, 0, 0, 0, expected_fp>;
    template<int64_t expected_fp> using ImpalaBloomLarge3 = FilterTestConfig<ImpalaBloom, impala::Standard, k,
            parameter::PowerOfTwoMurmurAVX232MT, n_l, s, 0, 4, 16, expected_fp>;


    /*
     * Test Types
     */

    using ImpalaBloomTestTypes = ::testing::Types<ImpalaBloomSmall1<64>, ImpalaBloomSmall2<48>, ImpalaBloomSmall3<12>,
            ImpalaBloomLarge1<65609>, ImpalaBloomLarge2<65479>, ImpalaBloomLarge3<10238>>;

}

