#pragma once

#include <parameter/parameter.hpp>
#include <benchmark/benchmark.h>
#include <stdexcept>

namespace filters {

    enum class FixtureType : size_t {
        Construct, Count, MTCount, Contains, Partitioning, FPR
    };

    enum class FixtureParameter : size_t {
        s = 0, n_threads = 1, n_partitions = 2, n_elements_build = 3, n_elements_lookup = 4, shared_elements = 5
    };

    template<FixtureType /*fixture*/, typename /*BenchmarkConfig*/>
    class Fixture : public ::benchmark::Fixture {

      public:
        void SetUp(const ::benchmark::State &/*state*/) override {
            throw std::runtime_error{"not implemented!"};
        }

        void TearDown(const ::benchmark::State &/*state*/) override {
            throw std::runtime_error{"not implemented!"};
        }

        void benchmark(::benchmark::State &/*state*/) {
            throw std::runtime_error{"not implemented!"};
        }
    };

} // filters
