#pragma once

#include <cstddef>
#include "../filter_test.hpp"

namespace test::fastfilter {

    namespace parameter = filters::parameter;
    namespace fastfilter = filters::fastfilter;

    static constexpr size_t s = 0; // cannot be configured by the user

    static constexpr size_t n_s = 1000, n_l = 1000000;

    static constexpr size_t k_s = 8, k_l = 16;

    static constexpr filters::FilterType FastfilterXor = filters::FilterType::FastfilterXor;

    static constexpr filters::FilterType FastfilterBloom = filters::FilterType::FastfilterBloom;

    /*
     * Test Types
     */

    template<filters::FilterType ft, template<size_t> typename FP> using FastfilterScalarSmall = FilterTestConfig<ft,
            FP, k_l, parameter::LemireMurmurScalar64, n_s, s, 0, 0, 0, -1>;
    template<filters::FilterType ft, template<size_t> typename FP> using FastfilterScalarLarge = FilterTestConfig<ft,
            FP, k_s, parameter::LemireMurmurScalar64MT, n_l, s, 0, 4, 16, -1>;

    /*
     * Variant Test Types
     */

    using FastfilterXorTestTypes = ::testing::Types<FastfilterScalarSmall<FastfilterXor, fastfilter::_xor::Standard>,
            FastfilterScalarSmall<FastfilterXor, fastfilter::_xor::CImplementation>,
            FastfilterScalarSmall<FastfilterXor, fastfilter::_xor::Compressed>,
            FastfilterScalarLarge<FastfilterXor, fastfilter::_xor::Standard>,
            FastfilterScalarLarge<FastfilterXor, fastfilter::_xor::CImplementation>,
            FastfilterScalarLarge<FastfilterXor, fastfilter::_xor::Compressed>>;

    using FastfilterBloomTestTypes = ::testing::Types<
            FastfilterScalarSmall<FastfilterBloom, fastfilter::bloom::Standard>,
            FastfilterScalarSmall<FastfilterBloom, fastfilter::bloom::Branchless>,
            FastfilterScalarLarge<FastfilterBloom, fastfilter::bloom::Standard>,
            FastfilterScalarLarge<FastfilterBloom, fastfilter::bloom::Branchless>>;
}


