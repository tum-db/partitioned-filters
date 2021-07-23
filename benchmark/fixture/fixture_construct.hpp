#pragma once

#include <cstddef>
#include <benchmark/benchmark.h>
#include <filter.hpp>
#include "fixture_base.hpp"
#include "../generator/generator.hpp"
#include "../perfbenchmark.hpp"

namespace filters {

    template<typename BenchmarkConfig>
    class Fixture<FixtureType::Construct, BenchmarkConfig> : public ::benchmark::Fixture {

      public:
        using B = BenchmarkConfig;
        using F = Filter<B::filter, typename B::FilterParameter, B::k, typename B::OptimizationParameter>;
        using V = filters::simd::Vector<B::OptimizationParameter::registerSize, B::OptimizationParameter::simd>;
        using G = Generator<B::generator, V>;

        std::unique_ptr<G> generator;

        void SetUp(const ::benchmark::State &state) override {
            size_t n_elements_build = state.range(static_cast<size_t>(FixtureParameter::n_elements_build));
            generator = std::make_unique<G>(n_elements_build, 0, 0);
        }

        void TearDown(const ::benchmark::State &) override {
        }

        void benchmark(::benchmark::State &state) {
            std::unique_ptr<F> filter;
            size_t s = state.range(static_cast<size_t>(FixtureParameter::s));
            size_t n_threads = state.range(static_cast<size_t>(FixtureParameter::n_threads));
            size_t n_partitions = state.range(static_cast<size_t>(FixtureParameter::n_partitions));

            perfBenchmark(state, [this, &filter, s, n_partitions, n_threads]() -> bool {
                filter = std::make_unique<F>(s, n_partitions, n_threads, 4 * n_threads);
                return filter->construct(generator->buildData(), generator->buildElements());
            });

            state.counters["bits"] =
                    static_cast<double >(filter->size() * 8) / static_cast<double>(generator->buildElements());
            state.counters["size"] = filter->size();
            state.counters["avg_size"] = filter->avg_size();
            state.counters["retries"] = filter->retries();
        }
    };
}
