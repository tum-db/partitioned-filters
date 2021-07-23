#pragma once

#include <parameter/parameter.hpp>
#include <benchmark/benchmark.h>
#include <stdexcept>

namespace filters {

    enum class GeneratorType : size_t {
        Sequential, Prime, Random, RandomHuge, RandomHuge2, Skew
    };

    template<GeneratorType /*generator*/, typename T>
    class Generator {

      public:
        Generator(size_t /*n_elements_build*/, size_t /*n_elements_lookup*/, size_t /*shared*/) {
            throw std::runtime_error{"not implemented!"};
        }

        T *buildData() {
            throw std::runtime_error{"not implemented!"};
        }

        T *lookupData() {
            throw std::runtime_error{"not implemented!"};
        }

        size_t buildElements() {
            throw std::runtime_error{"not implemented!"};
        }

        size_t lookupElements() {
            throw std::runtime_error{"not implemented!"};
        }

        size_t sharedElements() {
            throw std::runtime_error{"not implemented!"};
        }
    };

} // filters
