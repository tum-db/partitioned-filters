#pragma once

#include <vector>
#include <numeric>
#include <benchmark/benchmark.h>
#include <cmath>
#include <functional>
#include <perfevent.hpp>

namespace filters {

    const std::vector<std::string> perfCounters = {"duration", "instructions", "cycles", "L1D-misses", "L1I-misses",
                                                   "DTLB-misses", "ITLB-misses", "LLC-misses", "branch-misses",
                                                   "task-clock"};

    double sum(const std::vector<double> &v) {
        return std::accumulate(v.begin(), v.end(), 0.0);
    }

    double mean(const std::vector<double> &v) {
        if (v.empty()) {
            return std::nan("");
        }

        return sum(v) * (1.0 / v.size());
    }

    size_t perfBenchmark(::benchmark::State &state, const std::function<bool()> &function) {
        std::vector<PerfEvent> events;

        for (auto _ : state) {
            PerfEvent e;
            e.startCounters();

            bool success = function();

            e.stopCounters();
            state.SetIterationTime(e.getDuration() / 1000);
            if (success) {
                events.push_back(e);
            }
        }

        for (auto counter : perfCounters) {
            std::vector<double> v;
            std::transform(events.begin(), events.end(), std::back_inserter(v), [&counter](PerfEvent event) {
                if (counter == "duration") {
                    return event.getDuration();
                } else {
                    return event.getCounter(counter);
                }
            });

            state.counters[counter] = mean(v);
        }

        state.counters["failures"] = static_cast<double>(state.max_iterations - events.size());
        return events.size();
    }

    static std::atomic_size_t successes{0}, iterations{0};

    size_t perfBenchmarkParallel(::benchmark::State &state, const std::function<bool()> &function) {
        if (state.thread_index() == 0) {
            successes = 0;
            iterations = 0;
        }

        for (auto _ : state) {
            size_t iteration = iterations++;
            if (iteration < state.max_iterations) {
                bool success = function();
                if (success) {
                    successes++;
                }
            }
        }

        if (state.thread_index() == 0) {
            state.counters["failures"] = static_cast<double>(state.max_iterations - successes);
        }
        return successes;
    }
}
