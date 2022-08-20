#pragma once

#include <cstddef>
#include <gtest/gtest.h>
#include <random>
#include <filter.hpp>
#include <perfevent.hpp>

#ifdef NDEBUG

static constexpr size_t max_n = 1000000;

#else

static constexpr size_t max_n = 1000000;

#endif

/**
 * TestEnvironment
 */
class Environment : public testing::Environment {

    using V32 = filters::simd::Vector<filters::parameter::RegisterSize::_32bit, filters::parameter::SIMD::AVX512>;
    using V64 = filters::simd::Vector<filters::parameter::RegisterSize::_64bit, filters::parameter::SIMD::AVX512>;

    using T32 = typename V32::T;
    using T64 = typename V64::T;

  private:

    const uint32_t prime32{0x5F5B9F1};
    uint32_t last32{0};
    T32 *data32;
    std::default_random_engine generator32{42};

    const uint64_t prime64{0xF6B75AB2BC471C7};
    uint64_t last64{0};
    T64 *data64;
    std::default_random_engine generator64{42};

    std::binomial_distribution<int> distribution{14, 0.5};

  public:

    ~Environment() override = default;

    void SetUp() override {
        size_t size = 11 * max_n;
        data32 = filters::simd::valloc<V32>(filters::simd::valign<V32>(size * sizeof(T32)));
        data64 = filters::simd::valloc<V64>(filters::simd::valign<V64>(size * sizeof(T64)));

        for (size_t i = 0; i < size; i++) {
            data32[i] = last32;
            last32 += prime32 * (distribution(generator32) + 1); // add one to avoid duplicate

            data64[i] = last64;
            last64 += prime64 * (distribution(generator64) + 1); // add one to avoid duplicatess
        }
    }

    template<typename T>
    T *get_data() {
        if constexpr (sizeof(T) == 4) {
            return data32;
        } else {
            return data64;
        }
    }

    void TearDown() override {
        free(data32);
        free(data64);
    }
};

Environment *environment;

#define MAIN()                                              \
int main(int argc, char **argv) {                           \
    environment = new Environment();                        \
    testing::AddGlobalTestEnvironment(environment);         \
    ::testing::InitGoogleTest(&argc, argv);                 \
    int res = RUN_ALL_TESTS();                              \
    return res;                                             \
}                                                           \
int main(int, char**)
