#pragma once

#include <cstddef>
#include <string>
#include <math/math.hpp>

namespace filters::cuckoo {

    enum class Variant : size_t {
        Standard, Morton, CuckooOpt, MortonOpt
    };

    constexpr size_t optimal_associativity(size_t k, Variant variant, size_t associativity) {
        if (k < 1 or k > 32) {
            throw std::runtime_error{"k not supported!"};
        }

        switch (variant) {
            case Variant::CuckooOpt:
                if (k <= 4) {
                    return 8;
                } else if (k <= 8) {
                    return 4;
                } else if (k <= 15) {
                    return 3;
                } else {
                    return 2;
                }
            case Variant::MortonOpt:
                if (k <= 3) {
                    return 7;
                } else {
                    return 3;
                }
            default:
                return associativity;
        }
    }

    constexpr size_t
    optimal_buckets_per_block(size_t k, Variant variant, size_t associativity, size_t compression_ratio,
                              size_t ota_ratio) {
        if (k < 1 or k > 32) {
            throw std::runtime_error{"k not supported!"};
        }

        switch (variant) {
            case Variant::Morton: {
                size_t counter_size = math::get_number_of_bits(associativity + 1);
                return 512 * 100 / (associativity * compression_ratio * k + counter_size * 100 + ota_ratio);
            }
            case Variant::MortonOpt:
                if (k <= 2) {
                    return 64;
                } else if (k <= 3) {
                    return 56;
                } else if (k <= 8) {
                    return 64;
                } else if (k <= 9) {
                    return 48;
                } else if (k <= 13) {
                    return 32;
                } else if (k <= 14) {
                    return 23;
                } else if (k <= 15) {
                    return 21;
                } else if (k <= 17) {
                    return 22;
                } else if (k <= 20) {
                    return 16;
                } else if (k <= 21) {
                    return 12;
                } else if (k <= 24) {
                    return 12;
                } else if (k <= 25) {
                    return 14;
                } else if (k <= 26) {
                    return 8;
                } else if (k <= 27) {
                    return 11;
                } else if (k <= 28) {
                    return 12;
                } else if (k <= 29) {
                    return 8;
                } else if (k <= 30) {
                    return 12;
                } else if (k <= 31) {
                    return 8;
                } else if (k <= 32) {
                    return 12;
                }
            default:
                return 0;
        }
    }

    constexpr size_t
    optimal_ota_size(size_t k, Variant variant, size_t associativity, size_t buckets_per_block, size_t ota_ratio) {
        if (k < 1 or k > 32) {
            throw std::runtime_error{"k not supported!"};
        }

        switch (variant) {
            case Variant::Morton: {
                size_t counter_size = math::get_number_of_bits(associativity + 1);
                size_t slots_per_block =
                        (512 - buckets_per_block * counter_size - buckets_per_block * ota_ratio / 100) / k;
                size_t slots_size_in_bits = slots_per_block * k;
                size_t counters_size_in_bits = buckets_per_block * counter_size;
                return (ota_ratio == 0)
                       ? 0
                       : 512 - slots_size_in_bits - counters_size_in_bits;
            }
            case Variant::MortonOpt:
                switch (k) {
                    case 1:
                        return 32;
                    case 2:
                        return 16;
                    case 3:
                        return 8;
                    case 4:
                        return 16;
                    case 5:
                        return 8;
                    case 6:
                        return 8;
                    case 7:
                        return 8;
                    case 8:
                        return 8;
                    case 9:
                        return 8;
                    case 10:
                        return 8;
                    case 11:
                        return 8;
                    case 12:
                        return 4;
                    case 13:
                        return 4;
                    case 14:
                        return 4;
                    case 15:
                        return 4;
                    case 16:
                        return 4;
                    case 17:
                        return 8;
                    case 18:
                        return 8;
                    case 19:
                        return 4;
                    case 20:
                        return 0;
                    case 21:
                        return 4;
                    case 22:
                        return 4;
                    case 23:
                        return 4;
                    case 24:
                        return 8;
                    case 25:
                        return 8;
                    case 26:
                        return 2;
                    case 27:
                        return 4;
                    case 28:
                        return 12;
                    case 29:
                        return 2;
                    case 30:
                        return 8;
                    case 31:
                        return 0;
                    case 32:
                        return 8;
                    default:
                        return 0;
                }
            default:
                return 0;
        }
    }

    constexpr double optimal_load_factor(size_t k) {
        if (k >= 9) {
            return 1.15;
        } else if (k >= 4) {
            return 1.20;
        } else if (k >= 3) {
            return 1.50;
        } else {
            return 3.00;
        }
    }

    static size_t optimal_bucket_size(size_t k, size_t associativity) {
        return math::align_number(k * associativity, 8ul);
    }

    template<size_t k, Variant _variant, size_t _associativity, size_t _compression_ratio, size_t _ota_ratio>
    struct CuckooFilterParameter {
        static constexpr Variant variant = (_variant == Variant::Morton or _variant == Variant::MortonOpt)
                                           ? Variant::Morton
                                           : Variant::Standard;
        static constexpr size_t associativity = optimal_associativity(k, _variant, _associativity);

        static_assert(_compression_ratio >= 0 and _compression_ratio < 100, "invalid compression ratio");
        static_assert(_ota_ratio >= 0 and _ota_ratio <= 100, "invalid ota ratio");

        // Morton dynamic parameter only valid if variant == Morton
        static constexpr size_t buckets_per_block = optimal_buckets_per_block(k, _variant, associativity,
                _compression_ratio, _ota_ratio);
        static constexpr size_t ota_size = optimal_ota_size(k, _variant, associativity, buckets_per_block, _ota_ratio);

        static std::string to_string() {
            std::string s_variant;
            switch (variant) {
                case Variant::Standard:
                    s_variant = "Standard";
                    break;
                case Variant::Morton:
                    s_variant = "Morton";
                    break;
            }

            std::string s = "{";
            s += "\"variant\": \"" + s_variant + "\", ";
            s += "\"associativity\": \"" + std::to_string(associativity) + " slots\",";
            s += "\"buckets_per_block\": " + std::to_string(buckets_per_block) + ",";
            s += "\"ota_size\": " + std::to_string(ota_size) + "}";

            return s;
        }
    };

    /**
     * Optimal Cuckoo Filter
     */

    template<size_t k> using CuckooOpt = CuckooFilterParameter<k, Variant::CuckooOpt, 0, 0, 0>;
    template<size_t k> using MortonOpt = CuckooFilterParameter<k, Variant::MortonOpt, 0, 0, 0>;


    /**
     * Standard Cuckoo Filter
     */
    template<size_t k> using Standard1 = CuckooFilterParameter<k, Variant::Standard, 1, 0, 0>;
    template<size_t k> using Standard3 = CuckooFilterParameter<k, Variant::Standard, 3, 0, 0>;
    template<size_t k> using Standard4 = CuckooFilterParameter<k, Variant::Standard, 4, 0, 0>;
    template<size_t k> using Standard5 = CuckooFilterParameter<k, Variant::Standard, 5, 0, 0>;
    template<size_t k> using Standard2 = CuckooFilterParameter<k, Variant::Standard, 2, 0, 0>;
    template<size_t k> using Standard6 = CuckooFilterParameter<k, Variant::Standard, 6, 0, 0>;
    template<size_t k> using Standard7 = CuckooFilterParameter<k, Variant::Standard, 7, 0, 0>;
    template<size_t k> using Standard8 = CuckooFilterParameter<k, Variant::Standard, 8, 0, 0>;


    /**
     * Compressed Cuckoo Filter (Morton Filter)
     */

    #define Morton(Buckets, CompressionRate)                                                                                                      \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_0 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 0>;    \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_5 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 5>;    \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_10 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 10>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_15 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 15>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_20 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 20>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_25 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 25>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_30 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 30>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_35 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 35>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_40 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 40>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_45 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 45>;  \
    template<size_t k> using Morton##Buckets##_##CompressionRate##_50 = CuckooFilterParameter<k, Variant::Morton, Buckets, CompressionRate, 50>

    /**
     * #buckets = 2
     */

    Morton(2, 5);
    Morton(2, 10);
    Morton(2, 15);
    Morton(2, 20);
    Morton(2, 25);
    Morton(2, 30);
    Morton(2, 35);
    Morton(2, 40);
    Morton(2, 45);
    Morton(2, 50);
    Morton(2, 55);
    Morton(2, 60);
    Morton(2, 65);
    Morton(2, 70);
    Morton(2, 75);
    Morton(2, 80);
    Morton(2, 85);
    Morton(2, 90);
    Morton(2, 95);

    /**
     * #buckets = 3
     */

    Morton(3, 5);
    Morton(3, 10);
    Morton(3, 15);
    Morton(3, 20);
    Morton(3, 25);
    Morton(3, 30);
    Morton(3, 35);
    Morton(3, 40);
    Morton(3, 45);
    Morton(3, 50);
    Morton(3, 55);
    Morton(3, 60);
    Morton(3, 65);
    Morton(3, 70);
    Morton(3, 75);
    Morton(3, 80);
    Morton(3, 85);
    Morton(3, 90);
    Morton(3, 95);

    /**
     * #buckets = 7
     */

    Morton(7, 5);
    Morton(7, 10);
    Morton(7, 15);
    Morton(7, 20);
    Morton(7, 25);
    Morton(7, 30);
    Morton(7, 35);
    Morton(7, 40);
    Morton(7, 45);
    Morton(7, 50);
    Morton(7, 55);
    Morton(7, 60);
    Morton(7, 65);
    Morton(7, 70);
    Morton(7, 75);
    Morton(7, 80);
    Morton(7, 85);
    Morton(7, 90);
    Morton(7, 95);

    /**
     * #buckets = 15
     */

    Morton(15, 5);
    Morton(15, 10);
    Morton(15, 15);
    Morton(15, 20);
    Morton(15, 25);
    Morton(15, 30);
    Morton(15, 35);
    Morton(15, 40);
    Morton(15, 45);
    Morton(15, 50);
    Morton(15, 55);
    Morton(15, 60);
    Morton(15, 65);
    Morton(15, 70);
    Morton(15, 75);
    Morton(15, 80);
    Morton(15, 85);
    Morton(15, 90);
    Morton(15, 95);

} // filters::cuckoo
