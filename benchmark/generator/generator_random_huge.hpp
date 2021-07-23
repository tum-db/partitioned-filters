#pragma once

#include <stdexcept>
#include "generator_base.hpp"
#include "data_container.hpp"

namespace filters {

    template<typename V>
    class Generator<GeneratorType::RandomHuge, V> {

      private:
        using T = typename V::T;

        const size_t n_elements = 1000000000;

        size_t n_elements_build, n_elements_lookup, shared_elements;
        T *build_data, *lookup_data;
        T last = 0;
        std::uniform_int_distribution<T> distribution;
        std::mt19937 seed{42};

      public:
        Generator(size_t n_elements_build, size_t n_elements_lookup, size_t shared_elements_percentage)
                : n_elements_build(n_elements_build), n_elements_lookup(n_elements_lookup), shared_elements(0) {

            size_t stored_n_elements_build = n_elements;
            size_t stored_n_elements_lookup = (n_elements_lookup > 1)
                                              ? n_elements
                                              : 0;

            bool isEmpty = dataContainer.createIfNotExists<GeneratorType::RandomHuge, V>(stored_n_elements_build,
                    stored_n_elements_lookup, 0);
            build_data = dataContainer.buildData<GeneratorType::RandomHuge, V>(stored_n_elements_build,
                    stored_n_elements_lookup, 0);
            lookup_data = dataContainer.lookupData<GeneratorType::RandomHuge, V>(stored_n_elements_build,
                    stored_n_elements_lookup, 0);

            if (isEmpty) {
                std::mt19937 engine(seed());

                for (size_t j = 0; j < stored_n_elements_build; j++) {
                    T value = distribution(seed);
                    build_data[j] = value;
                }
                std::shuffle(build_data, build_data + stored_n_elements_build, engine);

                for (size_t j = shared_elements; j < stored_n_elements_lookup; j++) {
                    T value = distribution(seed);
                    lookup_data[j] = value;
                }
                std::shuffle(lookup_data, lookup_data + stored_n_elements_lookup, engine);
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
