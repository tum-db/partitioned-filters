#pragma once

#include <cstddef>
#include <xor/xor_parameter.hpp>
#include "../filter_pointer_test.hpp"

namespace test::nxor::pointer {

    namespace parameter = filters::parameter;
    namespace nxor = filters::nxor;

    static constexpr size_t s = 125;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr filters::FilterType Xor = filters::FilterType::Xor;


    /*
     * Large Test Types
     */

    template<size_t k, int64_t expected_fp> using XorScalarLarge1 = FilterPointerTestConfig<Xor, k,
            parameter::MagicMurmurScalar64, n_l, s, expected_fp>;
    template<size_t k, int64_t expected_fp> using XorScalarLarge2 = FilterPointerTestConfig<Xor, k,
            parameter::LemireMurmurScalar64, n_l, s, expected_fp>;


    /*
     * Variant Test Types
     */

    using XorTestTypes = ::testing::Types<XorScalarLarge1<32, 0>, XorScalarLarge2<31, 0>, XorScalarLarge1<30, 0>,
            XorScalarLarge2<29, 0>, XorScalarLarge1<28, 0>, XorScalarLarge2<27, 0>, XorScalarLarge1<26, 0>,
            XorScalarLarge2<25, 0>, XorScalarLarge1<24, 0>, XorScalarLarge2<23, 0>, XorScalarLarge1<22, 3>,
            XorScalarLarge2<21, 2>, XorScalarLarge1<20, 10>, XorScalarLarge2<19, 18>, XorScalarLarge1<18, 28>,
            XorScalarLarge2<17, 70>, XorScalarLarge1<16, 136>, XorScalarLarge2<15, 286>, XorScalarLarge1<14, 574>,
            XorScalarLarge2<13, 1156>, XorScalarLarge1<12, 2324>, XorScalarLarge2<11, 4776>, XorScalarLarge1<10, 9628>,
            XorScalarLarge2<9, 19230>, XorScalarLarge1<8, 38856>, XorScalarLarge2<7, 78146>, XorScalarLarge1<6, 155576>,
            XorScalarLarge2<5, 312786>, XorScalarLarge1<4, 624015>, XorScalarLarge2<3, 1250331>,
            XorScalarLarge1<2, 2499729>, XorScalarLarge2<1, 4999575>>;
}


