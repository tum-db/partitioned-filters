#pragma once

#include <vector>
#include <string>
#include <address/addresser.hpp>
#include <bloom/container/bloom_container.hpp>
#include <parameter/parameter.hpp>
#include <cmath>

namespace filters::fastfilter::_xor {

    enum class Variant : size_t {
        Standard, CImplementation, Compressed
    };

    template<Variant _variant>
    struct FastfilterXorParameter {
        static constexpr Variant variant = _variant;

        static std::string to_string() {
            std::string s_variant;
            switch (variant) {
                case Variant::Standard:
                    s_variant = "Standard";
                    break;
                case Variant::CImplementation:
                    s_variant = "CImplementation";
                    break;
                case Variant::Compressed:
                    s_variant = "Compressed";
                    break;
            }

            std::string s = "{";
            s += "\"variant\": \"" + s_variant + "\"}";

            return s;
        }
    };

    template<size_t> using Standard = FastfilterXorParameter<Variant::Standard>;
    template<size_t> using CImplementation = FastfilterXorParameter<Variant::CImplementation>;
    template<size_t> using Compressed = FastfilterXorParameter<Variant::Compressed>;

} // filters::fastfilter
