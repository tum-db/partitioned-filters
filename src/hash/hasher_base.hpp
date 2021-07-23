#pragma once

#include <cstddef>
#include <parameter/parameter.hpp>

namespace filters::hash {

    using HashingMode = parameter::HashingMode;

    template<HashingMode, typename Vector, size_t>
    struct Hasher {
    };

} // filters::hash
