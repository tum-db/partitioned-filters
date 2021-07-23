#pragma once

#include <cstddef>
#include <bloom/bloom_parameter.hpp>
#include "../filter_pointer_test.hpp"

namespace test::bloom::pointer {

    namespace bloom = filters::bloom;
    namespace parameter = filters::parameter;

    static constexpr size_t s = 144;

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr filters::FilterType Bloom = filters::FilterType::Bloom;


    /*
     * Small Test Types
     */

    template<size_t k, int64_t expected_fp> using BloomScalarSmall1 = FilterPointerTestConfig<Bloom, k,
            parameter::MagicMurmurScalar64, n_s, s, expected_fp>;
    template<size_t k, int64_t expected_fp> using BloomScalarSmall2 = FilterPointerTestConfig<Bloom, k,
            parameter::LemireMurmurScalar64, n_s, s, expected_fp>;

    /*
     * Large Test Types
     */

    template<size_t k, int64_t expected_fp> using BloomScalarLarge1 = FilterPointerTestConfig<Bloom, k,
            parameter::MagicMurmurScalar64, n_l, s, expected_fp>;
    template<size_t k, int64_t expected_fp> using BloomScalarLarge2 = FilterPointerTestConfig<Bloom, k,
            parameter::LemireMurmurScalar64, n_l, s, expected_fp>;


    /*
     * Variant Test Types
     */

    using BloomTestTypes = ::testing::Types<BloomScalarSmall1<16, 1>, BloomScalarSmall2<15, 0>,
            BloomScalarSmall1<14, 2>, BloomScalarSmall2<13, 0>, BloomScalarLarge1<12, 2409>,
            BloomScalarLarge2<11, 5013>, BloomScalarLarge1<10, 10036>, BloomScalarLarge2<9, 19550>,
            BloomScalarLarge1<8, 39471>, BloomScalarLarge2<7, 78874>, BloomScalarLarge1<6, 158143>,
            BloomScalarLarge2<5, 313692>, BloomScalarLarge1<4, 627864>, BloomScalarLarge2<3, 1253585>,
            BloomScalarLarge1<2, 2506664>, BloomScalarLarge2<1, 5002292>>;
}

