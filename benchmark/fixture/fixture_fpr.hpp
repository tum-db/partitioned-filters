#pragma once

#include <cstddef>
#include <benchmark/benchmark.h>
#include <filter.hpp>
#include "fixture_base.hpp"
#include "../generator/generator.hpp"
#include "../perfbenchmark.hpp"

namespace filters {

    template<typename BenchmarkConfig>
    class Fixture<FixtureType::FPR, BenchmarkConfig> : public ::benchmark::Fixture {

      public:
        using B = BenchmarkConfig;
        using F = Filter<B::filter, typename B::FilterParameter, B::k, typename B::OptimizationParameter>;
        using V = filters::simd::Vector<B::OptimizationParameter::registerSize, B::OptimizationParameter::simd>;
        using G = Generator<B::generator, V>;
        using PartitionSet = partition::PartitionSet<B::OptimizationParameter::partitioning, V>;

        std::unique_ptr<G> generator;
        std::atomic_size_t atomic_iteration{0}, false_positives{0}, elements{0};

        void SetUp(const ::benchmark::State &state) override {
            if (state.thread_index == 0) {
                size_t n_elements_build = state.range(static_cast<size_t>(FixtureParameter::n_elements_build));
                size_t n_elements_lookup = state.range(static_cast<size_t>(FixtureParameter::n_elements_lookup));
                size_t shared_elements = state.range(static_cast<size_t>(FixtureParameter::shared_elements));

                generator = std::make_unique<G>(n_elements_build * state.max_iterations, n_elements_lookup,
                        shared_elements);

                atomic_iteration = 0;
                false_positives = 0;
                elements = 0;
            }
        }

        void TearDown(const ::benchmark::State &) override {
        }

        void benchmark(::benchmark::State &state) {
            size_t filter_size, filter_avg_size, filter_retries;
            perfBenchmarkParallel(state, [this, &state, &filter_size, &filter_avg_size, &filter_retries]() {
                size_t iteration = atomic_iteration++;
                size_t s = state.range(static_cast<size_t>(FixtureParameter::s));
                size_t n_threads = state.range(static_cast<size_t>(FixtureParameter::n_threads));
                size_t n_partitions = state.range(static_cast<size_t>(FixtureParameter::n_partitions));
                size_t n_elements_build = state.range(static_cast<size_t>(FixtureParameter::n_elements_build));

                auto filter = std::make_unique<F>(s, n_partitions, n_threads, 4 * n_threads);
                bool success = filter->construct(generator->buildData() + n_elements_build * iteration,
                        n_elements_build);

                filter_size = filter->size();
                filter_avg_size = filter->avg_size();
                filter_retries = filter->retries();
                if (success) {
                    size_t counter = filter->count(generator->lookupData(), generator->lookupElements());
                    false_positives += counter - generator->sharedElements();
                    elements += generator->lookupElements() - generator->sharedElements();
                }

                return success;
            });

            if (state.thread_index == 0) {
                state.counters["fpr"] = (static_cast<double>(false_positives) / static_cast<double>(elements));
                state.counters["bits"] = static_cast<double>(filter_size * 8) / (static_cast<double>(state.range(
                        static_cast<size_t>(FixtureParameter::n_elements_build))));
                state.counters["size"] = filter_size;
                state.counters["avg_size"] = filter_avg_size;
                state.counters["retries"] = filter_retries;
            }
        }
    };
}
