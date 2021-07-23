#pragma once

#include <cstddef>
#include <string>

namespace filters::bsd::bloom {

    /*
     * Naive: standard bloom filter
     * Blocked: implements register-blocked, cache-blocked and sectorized (number of zones is always 1)
     * Zoned: implements grouped bloom filter (number of zones is identical to number of words)
     */
    enum class Variant : size_t {
        Naive, Blocked, Zoned
    };

    enum class Batching : bool {
        Enabled = true, Disabled = false
    };

    template<Variant _variant, size_t w, size_t sz, Batching _batching>
    struct BSDBloomParameter {
        static constexpr Variant variant = _variant;
        static constexpr size_t word_cnt_per_block = w;
        static constexpr size_t sector_zone_cnt = sz;
        static constexpr Batching batching = _batching;

        static std::string to_string() {
            std::string s_variant;
            switch (variant) {
                case Variant::Naive:
                    s_variant = "Naive";
                    break;
                case Variant::Blocked:
                    s_variant = "Blocked";
                    break;
                case Variant::Zoned:
                    s_variant = "Zoned";
                    break;
            }
            std::string s_batching = (static_cast<bool>(batching))
                                     ? "Enabled"
                                     : "Disabled";

            std::string s = "{";
            s += "\"variant\": \"" + s_variant + "\", ";
            s += "\"word_cnt_per_block\": " + std::to_string(word_cnt_per_block) + ", ";
            s += "\"sector_zone_cnt\": " + std::to_string(sector_zone_cnt) + ", ";
            s += "\"batching\": " + s_batching + "}";

            return s;
        }
    };

    template<size_t> using Naive32 = BSDBloomParameter<Variant::Naive, 0, 0, Batching::Disabled>;
    template<size_t> using Blocked32 = BSDBloomParameter<Variant::Blocked, 1, 1, Batching::Disabled>;
    template<size_t> using Blocked128 = BSDBloomParameter<Variant::Blocked, 4, 4, Batching::Disabled>;
    template<size_t> using Blocked256 = BSDBloomParameter<Variant::Blocked, 8, 1, Batching::Disabled>;
    template<size_t> using Blocked512 = BSDBloomParameter<Variant::Blocked, 16, 1, Batching::Disabled>;
    template<size_t> using Sectorized128 = BSDBloomParameter<Variant::Blocked, 4, 4, Batching::Disabled>;
    template<size_t> using Sectorized256 = BSDBloomParameter<Variant::Blocked, 8, 8, Batching::Disabled>;
    template<size_t> using Sectorized512 = BSDBloomParameter<Variant::Blocked, 16, 16, Batching::Disabled>;
    template<size_t> using Grouped2 = BSDBloomParameter<Variant::Zoned, 16, 2, Batching::Disabled>;
    template<size_t> using Grouped4 = BSDBloomParameter<Variant::Zoned, 16, 4, Batching::Disabled>;
    template<size_t> using Grouped8 = BSDBloomParameter<Variant::Zoned, 16, 8, Batching::Disabled>;

    template<size_t> using Naive32Batched = BSDBloomParameter<Variant::Naive, 0, 0, Batching::Enabled>;
    template<size_t> using Blocked32Batched = BSDBloomParameter<Variant::Blocked, 1, 1, Batching::Enabled>;
    template<size_t> using Blocked128Batched = BSDBloomParameter<Variant::Blocked, 4, 1, Batching::Enabled>;
    template<size_t> using Blocked256Batched = BSDBloomParameter<Variant::Blocked, 8, 1, Batching::Enabled>;
    template<size_t> using Blocked512Batched = BSDBloomParameter<Variant::Blocked, 16, 1, Batching::Enabled>;
    template<size_t> using Sectorized128Batched = BSDBloomParameter<Variant::Blocked, 4, 4, Batching::Enabled>;
    template<size_t> using Sectorized256Batched = BSDBloomParameter<Variant::Blocked, 8, 8, Batching::Enabled>;
    template<size_t> using Sectorized512Batched = BSDBloomParameter<Variant::Blocked, 16, 16, Batching::Enabled>;
    template<size_t> using Grouped2Batched = BSDBloomParameter<Variant::Zoned, 16, 2, Batching::Enabled>;
    template<size_t> using Grouped4Batched = BSDBloomParameter<Variant::Zoned, 16, 4, Batching::Enabled>;
    template<size_t> using Grouped8Batched = BSDBloomParameter<Variant::Zoned, 16, 8, Batching::Enabled>;

} // filters::bsd::bloom
