#pragma once

#include <cstddef>
#include <gtest/gtest.h>
#include <random>
#include <filter_pointer.hpp>
#include "tester.hpp"

template<filters::FilterType _type, size_t _k, typename OP, size_t _n, size_t _s, int64_t _expected_fp>
struct FilterPointerTestConfig {
    static constexpr filters::FilterType type = _type;
    static constexpr size_t k = _k;
    using OptimizationParameter = OP;
    static constexpr size_t n = _n;
    static constexpr size_t s = _s;
    static constexpr int64_t expected_fp = _expected_fp;

    static_assert(n <= max_n, "n is too large!");
};

template<typename TypeParam>
class FilterPointerTest : public ::testing::Test {

  public:
    static constexpr size_t k = TypeParam::k;
    static constexpr size_t n = TypeParam::n;
    static constexpr size_t s = TypeParam::s;
    static constexpr int64_t expected_fp = TypeParam::expected_fp;

    using Vector = filters::simd::Vector<TypeParam::OptimizationParameter::registerSize,
            TypeParam::OptimizationParameter::simd>;
    using T = typename Vector::T;

    T *data;
    size_t size = 11 * n;

    void SetUp() override {
        data = environment->template get_data<T>();
    }

    void TearDown() override {
    }

};

TYPED_TEST_CASE_P(FilterPointerTest);

TYPED_TEST_P(FilterPointerTest, ConstructAndCount) {
    filters::FilterPointer<typename TypeParam::OptimizationParameter> filter;
    filter.template create<TypeParam::type>(this->k, this->s);

    bool success = filter.add_all(this->data, this->n);
    ASSERT_TRUE(success) << "could not build filter";

    size_t false_negatives = 0;
    for (size_t i = 0; i < this->n; i++) {
        false_negatives += filter.contains(this->data[i]);
    }
    ASSERT_EQ(false_negatives, this->n) << "Filter has false negatives.";

    size_t false_positives = 0;
    for (size_t i = 0; i < this->size; i++) {
        false_positives += filter.contains(this->data[i]);
    }
    false_positives -= this->n;

    if (this->expected_fp >= 0) {
        ASSERT_EQ(false_positives, this->expected_fp) << "Number of false positive is not equal.";
    }

    double false_positive_rate = static_cast<double>(false_positives) / static_cast<double>(this->size - this->n);
    std::cout << "#false positives: " << false_positives << std::endl;
    std::cout << std::setprecision(10) << "false positive rate: " << false_positive_rate << std::endl;
}

REGISTER_TYPED_TEST_CASE_P(FilterPointerTest, ConstructAndCount);
