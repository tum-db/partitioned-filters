#pragma once

#include <cstddef>
#include <gtest/gtest.h>
#include <random>
#include <filter.hpp>
#include <perfevent.hpp>
#include <simd/vector_div.hpp>
#include "tester.hpp"

template<size_t _divisor>
struct DivModTestConfig {
    static constexpr size_t divisor = _divisor;
};

/**
 * Test
 * @tparam TypeParam
 */
template<typename TypeParam>
class DivModTest : public ::testing::Test {

  public:

    void SetUp() override {
    }

    void TearDown() override {
    }

};

TYPED_TEST_CASE_P(DivModTest);

TYPED_TEST_P(DivModTest, DivAndMod) {
    static constexpr size_t divisor = TypeParam::divisor;
    using Vector = filters::simd::Vector<filters::parameter::RegisterSize::_32bit, filters::parameter::SIMD::AVX512>;
    using T = typename Vector::T;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, std::numeric_limits<uint32_t>::max());

    for (size_t i = 0; i < 1000000; i++) {
        uint32_t v = dis(gen);
        Vector div, mod;
        filters::simd::div_mod<divisor>(Vector(v), div, mod);
        ASSERT_EQ(div.elements[0], v / divisor);
        ASSERT_EQ(mod.elements[0], v % divisor);
    }
}

REGISTER_TYPED_TEST_CASE_P(DivModTest, DivAndMod);
