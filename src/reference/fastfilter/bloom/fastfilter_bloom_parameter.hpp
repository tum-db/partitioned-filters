#pragma once

#include <vector>
#include <string>
#include <address/addresser.hpp>
#include <bloom/container/bloom_container.hpp>
#include <parameter/parameter.hpp>
#include <cmath>

namespace filters::fastfilter::bloom {

    template<bool _branchless>
    struct FastfilterBloomParameter {

        static constexpr bool branchless = _branchless;

        static std::string to_string() {
            std::string s_branchless = (branchless)
                                       ? "Enabled"
                                       : "Disabled";

            std::string s = "{";
            s += "\"branchless\": \"" + s_branchless + "\"}";
            return s;
        }
    };

    template<size_t> using Standard = FastfilterBloomParameter<false>;
    template<size_t> using Branchless = FastfilterBloomParameter<true>;

} // filters::fastfilter::bloom
