#pragma once

#include <cstdint>
#include <compiler/compiler_hints.hpp>
#include <cstddef>

struct KnuthLCG {

    // just initialize with zero, first iteration will generator a non-zero number
    // NOTE: all instances will produce the same numbers
    uint64_t seed{0};

    template<size_t n_bits>
    forceinline
    uint32_t rand() {
        // Donald Knuth's configuration for a Linear Congruential Generator
        // taken from https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
        seed = (6364136223846793005ull * seed + 1442695040888963407ull);
        return seed >> (64 - n_bits);
    }

    forceinline
    void reset() {
        seed = 0;
    }
};
