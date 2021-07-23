#pragma once

#include <cstddef>
#include <gtest/gtest.h>
#include <random>
#include <filter.hpp>
#include <perfevent.hpp>
#include "tester.hpp"

/**
 * TestConfig
 * @tparam _type
 * @tparam FP
 * @tparam _k
 * @tparam OP
 * @tparam _n
 * @tparam _s
 * @tparam _n_partitions
 * @tparam _expected_fp
 */
template<filters::FilterType _type, template<size_t> typename FP, size_t _k,
        typename OP, size_t _n, size_t _s, size_t _n_partitions, size_t _n_threads, size_t _n_tasks_per_level, int64_t _expected_fp>
struct FilterTestConfig {
    static constexpr filters::FilterType type = _type;
    using FilterParameter = FP<_k>;
    static constexpr size_t k = _k;
    using OptimizationParameter = OP;
    static constexpr size_t n = _n;
    static constexpr size_t s = _s;
    static constexpr size_t n_partitions = _n_partitions;
    static constexpr size_t n_threads = _n_threads;
    static constexpr size_t n_tasks_per_level = _n_tasks_per_level;
    static constexpr int64_t expected_fp = _expected_fp;

    static_assert(n <= max_n, "n is too large!");
};

/**
 * Test
 * @tparam TypeParam
 */
template<typename TypeParam>
class FilterTest : public ::testing::Test {

  public:
    static constexpr size_t n = TypeParam::n;
    static constexpr size_t s = TypeParam::s;
    static constexpr size_t n_partitions = TypeParam::n_partitions;
    static constexpr size_t n_threads = TypeParam::n_threads;
    static constexpr size_t n_tasks_per_level = TypeParam::n_tasks_per_level;
    static constexpr int64_t expected_fp = TypeParam::expected_fp;

    using Vector = filters::simd::Vector<TypeParam::OptimizationParameter::registerSize,
            TypeParam::OptimizationParameter::simd>;
    using T = typename Vector::T;
    using PartitionSet = filters::partition::PartitionSet<TypeParam::OptimizationParameter::partitioning, Vector>;

    T *data;
    size_t size = 11 * n;

    void SetUp() override {
        data = environment->template get_data<T>();
    }

    void TearDown() override {
    }

};

TYPED_TEST_CASE_P(FilterTest);

TYPED_TEST_P(FilterTest, ConstructAndCount) {
    using F = filters::Filter<TypeParam::type, typename TypeParam::FilterParameter, TypeParam::k,
            typename TypeParam::OptimizationParameter>;
    using Vector = filters::simd::Vector<TypeParam::OptimizationParameter::registerSize,
            TypeParam::OptimizationParameter::simd>;
    using PartitionSet = filters::partition::PartitionSet<TypeParam::OptimizationParameter::partitioning, Vector>;

    PartitionSet partitions(this->n_partitions);
    partitions.init(this->data, this->n);

    auto filter = std::make_unique<F>(this->s, this->n_partitions, this->n_threads, this->n_tasks_per_level);
    size_t false_positives = 0;

    // test batched interfaces
    {
        bool success = filter->construct(this->data, this->n);
        std::cout << filter->to_string() << std::endl;
        ASSERT_TRUE(success) << "Building filter has failed.";

        size_t false_negatives = filter->count(this->data, this->n);
        ASSERT_EQ(false_negatives, this->n) << "Filter has false negatives.";

        false_positives = filter->count(this->data, this->size) - this->n;

        if (this->expected_fp >= 0) {
            ASSERT_EQ(false_positives, this->expected_fp) << "Number of false positive is not equal.";
        }
    }

    if constexpr (F::supports_add) {
        std::cout << "test add" << std::endl;
        filter->init(partitions.histogram);

        bool success = true;
        for (size_t i = 0; i < this->n; i++) {
            success &= filter->add(this->data[i]);
        }
        ASSERT_TRUE(success) << "Building filter has failed.";

        size_t false_negatives = 0;
        for (size_t i = 0; i < this->n; i++) {
            false_negatives += filter->contains(this->data[i]);
        }
        ASSERT_EQ(false_negatives, this->n) << "Filter has false negatives.";
    }

    {
        false_positives = 0;

        for (size_t i = 0; i < this->size; i++) {
            false_positives += filter->contains(this->data[i]);
        }
        false_positives -= this->n;

        if (this->expected_fp >= 0) {
            ASSERT_EQ(false_positives, this->expected_fp) << "Number of false positive is not equal.";
        }
    }

    if constexpr (F::supports_add_partition) {
        std::cout << "test add_partition" << std::endl;
        filter->init(partitions.histogram);

        bool success = true;
        for (size_t i = 0; i < partitions.n_partitions; i++) {
            success &= filter->add_partition(partitions.values + partitions.offsets[i], partitions.histogram[i], i);
        }
        ASSERT_TRUE(success) << "Building filter has failed.";

        size_t false_negatives = 0;
        for (size_t i = 0; i < this->n; i++) {
            false_negatives += filter->contains(this->data[i]);
        }
        ASSERT_EQ(false_negatives, this->n) << "Filter has false negatives.";
    }

    {
        false_positives = filter->count(this->data, this->size) - this->n;

        if (this->expected_fp >= 0) {
            ASSERT_EQ(false_positives, this->expected_fp) << "Number of false positive is not equal.";
        }
    }

    double false_positive_rate = static_cast<double>(false_positives) / static_cast<double>(this->size - this->n);
    std::cout << "#false positives: " << false_positives << std::endl;
    std::cout << std::setprecision(10) << "false positive rate: " << false_positive_rate << std::endl;
}

REGISTER_TYPED_TEST_CASE_P(FilterTest, ConstructAndCount);
