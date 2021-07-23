#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include <parameter/parameter.hpp>
#include "barrier.hpp"

namespace filters::task {

    template<parameter::MultiThreading>
    class TaskQueue {

      public:
        
        TaskQueue(size_t, size_t) {
        }

        forceinline
        size_t get_n_threads() const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        size_t get_n_tasks_per_level() const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        void add_task(const std::function<void(size_t)> &) {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        void add_barrier() {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        void execute_tasks() {
            throw std::runtime_error{"not implemented!"};
        }
    };

    template<>
    class TaskQueue<parameter::MultiThreading::Enabled> {
      private:
        std::vector<std::function<void(size_t)>> tasks;
        std::atomic<size_t> pos{0};
        size_t n_threads, n_tasks_per_level;
        Barrier barrier;
        std::vector<std::thread> threads;

        forceinline
        static void set_cpu_affinity(int cpu_id) {
            cpu_set_t mask;
            CPU_ZERO(&mask);
            CPU_SET(cpu_id % std::thread::hardware_concurrency(), &mask);
            pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
        }

        forceinline
        bool execute_next_task(size_t thread_id) {
            size_t p = pos++;
            if (p >= tasks.size()) {
                return false;
            } else {
                tasks.at(p)(thread_id);
                return true;
            }
        }

        forceinline
        void reset() {
            tasks.clear();
            barrier.reset();
            threads.clear();
            pos = 0;
        }

      public:

        TaskQueue(size_t n_threads, size_t n_tasks_per_level) : n_threads(n_threads),
                                                                n_tasks_per_level(n_tasks_per_level),
                                                                barrier(n_threads) {
        }

        forceinline
        size_t get_n_threads() const {
            return n_threads;
        }

        forceinline
        size_t get_n_tasks_per_level() const {
            return n_tasks_per_level;
        }

        forceinline
        void add_task(const std::function<void(size_t)> &f) {
            tasks.push_back(f);
        }

        forceinline
        void add_barrier() {
            for (size_t i = 0; i < n_threads; i++) {
                tasks.emplace_back([&](size_t) {
                    barrier.wait();
                });
            }
        }

        forceinline
        void execute_tasks() {
            for (size_t i = 0; i < n_threads; i++) {
                threads.emplace_back([i, this] {
                    set_cpu_affinity(i);
                    while (this->execute_next_task(i));
                });
            }

            for (auto &thread : threads) {
                thread.join();
            }

            reset();
        }
    };
}  // namespace filters::task
