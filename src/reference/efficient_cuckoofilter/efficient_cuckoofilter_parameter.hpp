#pragma once

#include <string>

namespace filters::efficientcuckoofilter {

    template<bool _semiSorted>
    struct EfficientCuckoofilterParameter {

        static constexpr bool semiSorted = _semiSorted;

        static std::string to_string() {
            std::string s_semiSorted = (semiSorted)
                                       ? "Enabled"
                                       : "Disabled";

            std::string s = "{";
            s += "\"semiSorted\": \"" + s_semiSorted + "\"}";
            return s;
        }
    };

    template<size_t> using Standard = EfficientCuckoofilterParameter<false>;
    template<size_t> using SemiSorted = EfficientCuckoofilterParameter<true>;

} // filters::efficientcuckoofilter
