#pragma once

#include <cstddef>
#include <string>

namespace filters::bsd::cuckoo {

    enum class Batching : bool {
        Enabled = true, Disabled = false
    };

    enum class Associativity : size_t {
        _1slot = 1, _2slot = 2, _4slot = 4
    };

    template<Associativity _associativity, Batching _batching>
    struct BSDCuckooParameter {
        static constexpr Associativity associativity = _associativity;
        static constexpr Batching batching = _batching;

        static std::string to_string() {
            std::string s_batching = (static_cast<bool>(batching))
                                     ? "Enabled"
                                     : "Disabled";

            std::string s = "{";
            s += "\"associativity\": " + std::to_string(static_cast<size_t>(associativity)) + ", ";
            s += "\"batching\": " + s_batching + "}";

            return s;
        }
    };

    template<size_t> using Standard1 = BSDCuckooParameter<Associativity::_1slot, Batching::Disabled>;
    template<size_t> using Standard2 = BSDCuckooParameter<Associativity::_2slot, Batching::Disabled>;
    template<size_t> using Standard4 = BSDCuckooParameter<Associativity::_4slot, Batching::Disabled>;

    template<size_t> using Standard1Batched = BSDCuckooParameter<Associativity::_1slot, Batching::Enabled>;
    template<size_t> using Standard2Batched = BSDCuckooParameter<Associativity::_2slot, Batching::Enabled>;
    template<size_t> using Standard4Batched = BSDCuckooParameter<Associativity::_4slot, Batching::Enabled>;

} // filters::bsd::cuckoo
