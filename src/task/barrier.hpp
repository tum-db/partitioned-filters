#pragma once

#include <condition_variable>
#include <cstddef>
#include <compiler/compiler_hints.hpp>

namespace filters::task {

    class Barrier {
      private:
        std::mutex latch;
        std::condition_variable cond;

        size_t n_threads;
        size_t counter;
        size_t generation{0};

      public:
        explicit Barrier(size_t n_threads) : n_threads(n_threads), counter(n_threads) {
        }

        forceinline
        void wait() {
            std::unique_lock<std::mutex> lock(latch);
            auto last_generation = generation;

            if (0 == --counter) {
                generation++;
                counter = n_threads;
                cond.notify_all();
            } else {
                cond.wait(lock, [this, last_generation] {
                    return last_generation != this->generation;
                });
            }
        }

        forceinline
        void reset() {
            generation = 0;
            counter = n_threads;
        }
    };

}  // namespace task
