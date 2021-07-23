#pragma once

#include <cstddef>
#include <string>

namespace filters::amdmortonfilter {

    enum class Associativity : size_t {
        _1slot = 1, _3slot = 3, _7slot = 7, _15slot = 15
    };

    template<Associativity _associativity>
    struct AMDMortonFilterParameter {
        static constexpr Associativity associativity = _associativity;

        static std::string to_string() {
            std::string s = "{";
            s += "\"associativity\": \"" + std::to_string(static_cast<size_t >(associativity)) + " slots\"}";
            return s;
        }
    };

    template<size_t> using Standard1 = AMDMortonFilterParameter<Associativity::_1slot>;
    template<size_t> using Standard3 = AMDMortonFilterParameter<Associativity::_3slot>;
    template<size_t> using Standard7 = AMDMortonFilterParameter<Associativity::_7slot>;
    template<size_t> using Standard15 = AMDMortonFilterParameter<Associativity::_15slot>;

} // filters::amdmortonfilter
