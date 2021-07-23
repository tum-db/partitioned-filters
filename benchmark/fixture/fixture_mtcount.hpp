#pragma once

#include <cstddef>
#include <benchmark/benchmark.h>
#include <filter.hpp>
#include "fixture_base.hpp"
#include "../generator/generator.hpp"
#include "../perfbenchmark.hpp"

namespace filters {

    template<typename BenchmarkConfig>
    class Fixture<FixtureType::MTCount, BenchmarkConfig> : public ::benchmark::Fixture {

      public:
        using B = BenchmarkConfig;
        using F = Filter<B::filter, typename B::FilterParameter, B::k, typename B::OptimizationParameter>;
        using V = filters::simd::Vector<B::OptimizationParameter::registerSize, B::OptimizationParameter::simd>;
        using G = Generator<B::generator, V>;

        static_assert(B::OptimizationParameter::multiThreading == parameter::MultiThreading::Disabled);

        std::unique_ptr<G> generator;
        std::unique_ptr<F> filter;
        bool success;

        void SetUp(const ::benchmark::State &state) override {
            size_t s = state.range(static_cast<size_t>(FixtureParameter::s));
            size_t n_threads = state.range(static_cast<size_t>(FixtureParameter::n_threads));
            size_t n_partitions = state.range(static_cast<size_t>(FixtureParameter::n_partitions));
            size_t n_elements_build = state.range(static_cast<size_t>(FixtureParameter::n_elements_build));
            size_t n_elements_lookup = state.range(static_cast<size_t>(FixtureParameter::n_elements_lookup));
            size_t shared_elements = state.range(static_cast<size_t>(FixtureParameter::shared_elements));

            generator = std::make_unique<G>(n_elements_build, n_elements_lookup, shared_elements);

            filter = std::make_unique<F>(s, n_partitions, n_threads, 4 * n_threads);
            success = filter->construct(generator->buildData(), n_elements_build);
        }

        void TearDown(const ::benchmark::State &) override {
        }

        void benchmark(::benchmark::State &state) {

            if (not success) {
                perfBenchmark(state, [this]() {
                    return false;
                });
                state.counters["fpr"] = -1;
            } else {
                size_t counter = 0;
                size_t n_threads = state.range(static_cast<size_t>(FixtureParameter::n_threads));
                perfBenchmark(state, [this, &counter, n_threads]() {
                    task::TaskQueue<parameter::MultiThreading::Enabled> queue(n_threads, 4 * n_threads);
                    std::atomic<size_t> amo_counter{0};

                    auto *data = generator->lookupData();
                    auto length = generator->lookupElements();

                    size_t n_tasks = queue.get_n_tasks_per_level();
                    for (; n_tasks > 0; n_tasks--) {
                        size_t n_elements = length / n_tasks;
                        queue.add_task([this, &amo_counter, data, n_elements](size_t thread) {
                            amo_counter += filter->count(data, n_elements);
                        });
                        data += n_elements;
                        length -= n_elements;
                    }
                    queue.execute_tasks();
                    counter = amo_counter;

                    return true;
                });

                // do not include first 10% of data as they always hit
                state.counters["fpr"] = static_cast<double>(counter - generator->sharedElements()) /
                                        static_cast<double>(generator->lookupElements() - generator->sharedElements());
            }
            state.counters["bits"] =
                    static_cast<double >(filter->size() * 8) / static_cast<double>(generator->buildElements());
            state.counters["size"] = filter->size();
            state.counters["avg_size"] = filter->avg_size();
            state.counters["retries"] = filter->retries();
        }
    };
}
