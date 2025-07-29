#pragma once

#include <cstddef>
#include <string>
#include <math/math.hpp>

namespace filters::maskbloom {

    enum class Variant : size_t {
        Mask4 = 4
    };

    enum class MaskCount : size_t {
        _1024masks = 1024, _2048masks = 2048
    };

    template <Variant _variant, MaskCount _maskCount>
    struct MaskBloomFilterParameter {
        static constexpr Variant variant = _variant;
        static constexpr MaskCount maskCount = _maskCount;

        static std::string to_string() {
            std::string s_variant;
            switch (variant) {
                case Variant::Mask4:
                    s_variant = "Mask4";
                    break;
            }

            std::string s = "{";
            s += "\"variant\": \"" + s_variant + "\", ";
            s += "\"mask_count\": " + std::to_string(static_cast<size_t>(maskCount)) + "}";

            return s;
        }
    };

    /**
     * Optimal Cuckoo Filter
     */

    template <size_t k>
    using MaskBloom4 = MaskBloomFilterParameter<Variant::Mask4, MaskCount::_2048masks>;

} // filters::maskbloom
