#pragma once

#include <stdexcept>
#include "generator_base.hpp"
#include "data_container.hpp"

namespace filters {

    template<typename V>
    class Generator<GeneratorType::Random, V> {

      private:
        using T = typename V::T;

        size_t n_elements_build, n_elements_lookup, shared_elements;
        T *build_data, *lookup_data;
        T last = 0;
        std::uniform_int_distribution<T> distribution;
        std::mt19937 seed{42};

      public:
        Generator(size_t n_elements_build, size_t n_elements_lookup, size_t shared_elements_percentage)
                : n_elements_build(n_elements_build), n_elements_lookup(n_elements_lookup),
                  shared_elements(n_elements_lookup * shared_elements_percentage / 100) {

            bool isEmpty = dataContainer.createIfNotExists<GeneratorType::Random, V>(n_elements_build,
                    n_elements_lookup, shared_elements_percentage);
            build_data = dataContainer.buildData<GeneratorType::Random, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);
            lookup_data = dataContainer.lookupData<GeneratorType::Random, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);

            if (isEmpty) {
                std::unordered_set<T> s;
                s.reserve(n_elements_build);
                std::mt19937 engine(seed());

                for (size_t j = 0; j < n_elements_build; j++) {
                    T value = distribution(seed);
                    build_data[j] = value;
                    s.insert(value);
                }
                std::shuffle(build_data, build_data + n_elements_build, engine);

                for (size_t j = 0; j < shared_elements; j++) {
                    lookup_data[j] = build_data[engine() % n_elements_build];
                }
                for (size_t j = shared_elements; j < n_elements_lookup;) {
                    T value = distribution(seed);
                    if (not s.contains(value)) {
                        lookup_data[j] = value;
                        j++;
                    }
                }
                std::shuffle(lookup_data, lookup_data + n_elements_lookup, engine);
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
