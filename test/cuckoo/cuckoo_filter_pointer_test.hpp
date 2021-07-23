#pragma once

#include <cstddef>
#include <xor/xor_parameter.hpp>
#include "../filter_pointer_test.hpp"

namespace test::cuckoo::pointer {

    namespace parameter = filters::parameter;
    namespace cuckoo = filters::cuckoo;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr filters::FilterType Cuckoo = filters::FilterType::Cuckoo;


    /*
     * Large Test Types
     */

    template<size_t k, size_t s, int64_t expected_fp> using CuckooScalarLarge1 = FilterPointerTestConfig<Cuckoo, k,
            parameter::MagicMurmurScalar64, n_l, s, expected_fp>;
    template<size_t k, size_t s, int64_t expected_fp> using CuckooScalarLarge2 = FilterPointerTestConfig<Cuckoo, k,
            parameter::LemireMurmurScalar64, n_l, s, expected_fp>;


    /*
     * Variant Test Types
     */

    using CuckooTestTypes = ::testing::Types<CuckooScalarLarge1<32, 115, 0>, CuckooScalarLarge2<31, 115, 0>,
            CuckooScalarLarge1<30, 115, 0>, CuckooScalarLarge2<29, 116, 0>, CuckooScalarLarge1<28, 115, 0>,
            CuckooScalarLarge2<27, 115, 0>, CuckooScalarLarge1<26, 115, 0>, CuckooScalarLarge2<25, 116, 1>,
            CuckooScalarLarge1<24, 115, 1>, CuckooScalarLarge2<23, 116, 5>, CuckooScalarLarge1<22, 115, 7>,
            CuckooScalarLarge2<21, 115, 15>, CuckooScalarLarge1<20, 115, 35>, CuckooScalarLarge2<19, 115, 52>,
            CuckooScalarLarge1<18, 115, 135>, CuckooScalarLarge2<17, 115, 254>, CuckooScalarLarge1<16, 116, 548>,
            CuckooScalarLarge2<15, 115, 1557>, CuckooScalarLarge1<14, 115, 3069>, CuckooScalarLarge2<13, 115, 6322>,
            CuckooScalarLarge1<12, 116, 12619>, CuckooScalarLarge2<11, 116, 25152>, CuckooScalarLarge1<10, 115, 50728>,
            CuckooScalarLarge2<9, 115, 102260>, CuckooScalarLarge1<8, 120, 258974>, CuckooScalarLarge2<7, 120, 515339>,
            CuckooScalarLarge1<6, 110, 1103589>, CuckooScalarLarge2<5, 110, 2135122>,
            CuckooScalarLarge1<4, 110, 6267912>, CuckooScalarLarge2<3, 130, 8277284>,
            CuckooScalarLarge1<2, 200, 9231371>, CuckooScalarLarge2<1, 400, 9816249>>;
}


