#pragma once

#include <vector>
#include <string>
#include <address/addresser.hpp>
#include <bloom/container/bloom_container.hpp>
#include <parameter/parameter.hpp>
#include <cmath>

namespace filters::nxor {

    static constexpr size_t RF0 = 0, RF1 = 21, RF2 = 42;

    static constexpr size_t FUSE_SEGMENTS = 128;

    enum class Variant : size_t {
        Standard, Fuse
    };

    template<Variant _variant>
    struct XorParameter {
        static constexpr size_t max_n_retries = 16;
        static constexpr Variant variant = _variant;

        static std::string to_string() {
            std::string s_variant;
            switch (variant) {
                case Variant::Standard:
                    s_variant = "Standard";
                    break;
                case Variant::Fuse:
                    s_variant = "Fuse";
                    break;
            }

            std::string s = "{";
            s += "\"variant\": \"" + s_variant + "\", ";
            s += "\"max_n_retries\": " + std::to_string(max_n_retries) + "}";
            return s;
        }
    };

    template<size_t> using Standard = XorParameter<Variant::Standard>;
    template<size_t> using Fuse = XorParameter<Variant::Fuse>;

} // filters::xor
