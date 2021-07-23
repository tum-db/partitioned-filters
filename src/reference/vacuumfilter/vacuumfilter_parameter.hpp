#pragma once

#include <string>

namespace filters::vacuumfilter {

    template<bool _semiSorted>
    struct VacuumFilterParameter {

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

    template<size_t> using Standard = VacuumFilterParameter<false>;
    template<size_t> using SemiSorted = VacuumFilterParameter<true>;

} // filters::vacuumfilter
