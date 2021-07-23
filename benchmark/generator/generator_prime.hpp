#pragma once

#include <stdexcept>
#include "generator_base.hpp"
#include "data_container.hpp"

namespace filters {

    template<typename V>
    class Generator<GeneratorType::Prime, V> {

      private:
        using T = typename V::T;
        static constexpr uint32_t prime32{0x5F5B9F1};
        static constexpr uint64_t prime64{0xF6B75AB2BC471C7};
        static constexpr T increment = (V::registerSize == parameter::RegisterSize::_32bit)
                                       ? prime32
                                       : prime64;

        size_t n_elements_build, n_elements_lookup, shared_elements;
        T *build_data, *lookup_data;
        T last = 0;
        std::binomial_distribution<int> distribution{14, 0.5};
        std::mt19937 seed{42};

      public:
        Generator(size_t n_elements_build, size_t n_elements_lookup, size_t shared_elements_percentage)
                : n_elements_build(n_elements_build), n_elements_lookup(n_elements_lookup),
                  shared_elements(n_elements_lookup * shared_elements_percentage / 100) {

            bool isEmpty = dataContainer.createIfNotExists<GeneratorType::Prime, V>(n_elements_build,
                    n_elements_lookup, shared_elements_percentage);
            build_data = dataContainer.buildData<GeneratorType::Prime, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);
            lookup_data = dataContainer.lookupData<GeneratorType::Prime, V>(n_elements_build, n_elements_lookup,
                    shared_elements_percentage);

            if (isEmpty) {
                std::mt19937 engine(seed());

                for (size_t j = 0; j < n_elements_build; j++) {
                    build_data[j] = last;
                    last += increment * (distribution(engine) + 1); // add one to avoid duplicate
                }
                std::shuffle(build_data, build_data + n_elements_build, engine);

                for (size_t j = 0; j < shared_elements; j++) {
                    lookup_data[j] = build_data[engine() % n_elements_build];
                }
                for (size_t j = shared_elements; j < n_elements_lookup; j++) {
                    lookup_data[j] = last;
                    last += increment * (distribution(engine) + 1); // add one to avoid duplicate
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
