#pragma once

#include <cstddef>
#include <benchmark/benchmark.h>
#include <filter.hpp>
#include "fixture_base.hpp"
#include "../generator/generator.hpp"
#include "../perfbenchmark.hpp"

namespace filters {

    template<typename BenchmarkConfig>
    class Fixture<FixtureType::Partitioning, BenchmarkConfig> : public ::benchmark::Fixture {

      public:
        using B = BenchmarkConfig;
        using V = filters::simd::Vector<B::OptimizationParameter::registerSize, B::OptimizationParameter::simd>;
        using G = Generator<B::generator, V>;
        static constexpr parameter::Partitioning p = B::OptimizationParameter::partitioning;
        static constexpr parameter::MultiThreading m = B::OptimizationParameter::multiThreading;
        static_assert(p == parameter::Partitioning::Enabled, "Partitioning must be enabled!");

        std::unique_ptr<G> generator;

        void SetUp(const ::benchmark::State &state) override {
            size_t n_elements_build = state.range(static_cast<size_t>(FixtureParameter::n_elements_build));
            generator = std::make_unique<G>(n_elements_build, 0, 0);
        }

        void TearDown(const ::benchmark::State &) override {
        }

        void benchmark(::benchmark::State &state) {
            size_t n_elements;
            size_t n_partitions = state.range(static_cast<size_t>(FixtureParameter::n_partitions));

            if (m == parameter::MultiThreading::Disabled) {
                perfBenchmark(state, [this, &n_elements, n_partitions]() {
                    n_elements = 0;
                    partition::PartitionSet<p, V> partitions(n_partitions);
                    partitions.init(generator->buildData(), generator->buildElements());

                    for (size_t i = 0; i < partitions.n_partitions; i++) {
                        n_elements += partitions.histogram[i];
                    }
                    return true;
                });
            } else {
                size_t n_threads = state.range(static_cast<size_t>(FixtureParameter::n_threads));
                task::TaskQueue<B::OptimizationParameter::multiThreading> queue(n_threads, n_threads * 4);

                perfBenchmark(state, [this, &queue, &n_elements, n_partitions]() {
                    n_elements = 0;
                    partition::PartitionSet<p, V> partitions(n_partitions);
                    partitions.init(generator->buildData(), generator->buildElements(), queue);
                    queue.execute_tasks();

                    for (size_t i = 0; i < partitions.n_partitions; i++) {
                        n_elements += partitions.histogram[i];
                    }
                    return true;
                });
            }

            state.counters["size"] = n_elements * sizeof(typename V::T);
            state.counters["avg_size"] = n_elements * sizeof(typename V::T) / n_partitions;
        }
    };
}
