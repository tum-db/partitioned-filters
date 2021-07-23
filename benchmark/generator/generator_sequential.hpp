#pragma once

#include <stdexcept>
#include "generator_base.hpp"

namespace filters {

    template<typename V>
    class Generator<GeneratorType::Sequential, V> {

      private:
        using T = typename V::T;

        size_t n_elements_build, n_elements_lookup, shared_elements;
        T *build_data, *lookup_data;
        size_t i = 0;

      public:
        Generator(size_t n_elements_build, size_t n_elements_lookup, size_t shared_elements_percentage)
                : n_elements_build(n_elements_build), n_elements_lookup(n_elements_lookup),
                  shared_elements(n_elements_lookup * shared_elements_percentage / 100) {

            std::mt19937 engine(42);

            build_data = simd::valloc<V>(simd::valign<V>(n_elements_build * sizeof(T)));
            for (size_t j = 0; j < n_elements_build; j++, i++) {
                build_data[j] = i;
            }
            std::shuffle(build_data, build_data + n_elements_build, engine);

            lookup_data = simd::valloc<V>(simd::valign<V>(n_elements_lookup * sizeof(T)));
            for (size_t j = 0; j < shared_elements; j++) {
                lookup_data[j] = build_data[engine() % n_elements_build];
            }
            for (size_t j = shared_elements; j < n_elements_lookup; j++, i++) {
                lookup_data[j] = i;
            }
            std::shuffle(lookup_data, lookup_data + n_elements_lookup, engine);
        }

        ~Generator() {
            free(build_data);
            free(lookup_data);
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
