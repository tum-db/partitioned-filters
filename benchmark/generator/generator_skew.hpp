#pragma once

#include <stdexcept>
#include "generator_base.hpp"
#include "data_container.hpp"

namespace filters {

    template<typename V>
    class Generator<GeneratorType::Skew, V> {

      private:
        using T = typename V::T;

        size_t n_elements_build, n_elements_lookup, shared_elements, zero_bits;
        T *build_data, *lookup_data;
        T last = 0;
        std::uniform_int_distribution<T> distribution;
        std::mt19937 seed{42};

      public:
        Generator(size_t n_elements_build, size_t n_elements_lookup, size_t shared_elements_percentage)
                : n_elements_build(n_elements_build), n_elements_lookup(n_elements_lookup), shared_elements(0),
                  zero_bits(shared_elements_percentage) {

            bool isEmpty = dataContainer.createIfNotExists<GeneratorType::Skew, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);
            build_data = dataContainer.buildData<GeneratorType::Skew, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);
            lookup_data = dataContainer.lookupData<GeneratorType::Skew, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);

            if (isEmpty) {
                size_t build_counter = 0;

                for (size_t j = 0; j < n_elements_build; j++) {
                    T value = build_counter;
                    build_counter += 1ull << zero_bits;
                    build_data[j] = value;
                }

                size_t lookup_counter = build_counter;

                for (size_t j = shared_elements; j < n_elements_lookup; j++) {
                    if (lookup_counter < build_counter) {
                        lookup_counter = build_counter;
                    }
                    T value = lookup_counter;
                    lookup_counter += 1ull << zero_bits;
                    lookup_data[j] = value;
                }
            }

            // fill caches
            uint64_t acc = 0;
            for (size_t i = 0; i < 2; i++) {
                for (size_t j = 0; j < n_elements_build; j++) {
                    acc += build_data[j];
                }
                for (size_t j = 0; j < n_elements_lookup; j++) {
                    acc += lookup_data[j];
                }
            }
            // prevent gcc from optimizing loops
            benchmark::DoNotOptimize(acc);
        }

        T *buildData() {
            return build_data;
        }

        T *lookupData() {
            return lookup_data;
        }

        size_t buildElements() {
            return n_elements_build;
        }

        size_t lookupElements() {
            return n_elements_lookup;
        }

        size_t sharedElements() {
            return shared_elements;
        }
    };

} // filters
