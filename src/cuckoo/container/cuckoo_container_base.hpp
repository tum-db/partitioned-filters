#pragma once

#include <cuckoo/cuckoo_parameter.hpp>
#include <parameter/parameter.hpp>

namespace filters::cuckoo {

    // thread local random number generator to avoid cache evictions
    static thread_local KnuthLCG rng{};

    template<bool /*retry*/, Variant, size_t /*associativity*/, size_t /*buckets_per_block*/, size_t /*ota_size*/, size_t /*k*/, parameter::Partitioning, parameter::RegisterSize, parameter::SIMD, parameter::AddressingMode, parameter::HashingMode>
    struct CuckooContainer {
    };

} // filters::bloom
