#pragma once

namespace filters::bloom {

    enum class Variant : size_t {
        Naive, Blocked, Sectorized, Grouped
    };

    enum class BlockSize : size_t {
        _512bit = 512, _256bit = 256, _128bit = 128, _64bit = 64, _32bit = 32
    };

    enum class Groups : size_t {
        _1group = 1, _2groups = 2, _4groups = 4, _8groups = 8
    };

    enum class Vectorization : size_t {
        Horizontal, Vertical
    };

    template<Variant _variant, Vectorization _vectorization, BlockSize _blockSize, Groups _groups>
    struct BloomFilterParameter {
        static constexpr Variant variant = _variant;
        static constexpr Vectorization vectorization = _vectorization;
        static constexpr BlockSize blockSize = _blockSize;
        static constexpr Groups groups = _groups;

        static std::string to_string() {
            std::string s_variant;
            switch (variant) {
                case Variant::Naive:
                    s_variant = "Naive";
                    break;
                case Variant::Blocked:
                    s_variant = "Blocked";
                    break;
                case Variant::Sectorized:
                    s_variant = "Sectorized";
                    break;
                case Variant::Grouped:
                    s_variant = "Grouped";
                    break;
            }

            std::string s_vectorization = (vectorization == Vectorization::Horizontal)
                                          ? "Horizontal"
                                          : "Vertical";

            std::string s = "{";
            s += "\"variant\": \"" + s_variant + "\", ";
            s += "\"vectorization\": \"" + s_vectorization + "\", ";
            s += "\"block_size\": \"" + std::to_string(static_cast<size_t >(blockSize)) + " bits\", ";
            s += "\"groups\": " + std::to_string(static_cast<size_t >(groups)) + "}";

            return s;
        }
    };

    /*
     * Perf relevant:
     * Naive64,
     * Blocked64,
     * Blocked512,
     * Sectorized128 (k * 2), Sectorized256 (k*4), Sectorized512 (k*8),
     * Grouped2 (k*2), Grouped4 (k*4),
     * Sectorized128Vertical, Sectorized256Vertical, Sectorized512Vertical
     */
    template<size_t> using Naive32 = BloomFilterParameter<Variant::Naive, Vectorization::Horizontal, BlockSize::_32bit,
            Groups::_1group>;
    template<size_t> using Naive64 = BloomFilterParameter<Variant::Naive, Vectorization::Horizontal, BlockSize::_64bit,
            Groups::_1group>;

    template<size_t> using Blocked32 = BloomFilterParameter<Variant::Sectorized, Vectorization::Horizontal,
            BlockSize::_32bit, Groups::_1group>;
    template<size_t> using Blocked64 = BloomFilterParameter<Variant::Sectorized, Vectorization::Horizontal,
            BlockSize::_64bit, Groups::_1group>;

    template<size_t> using Blocked128 = BloomFilterParameter<Variant::Blocked, Vectorization::Horizontal,
            BlockSize::_128bit, Groups::_1group>;
    template<size_t> using Blocked256 = BloomFilterParameter<Variant::Blocked, Vectorization::Horizontal,
            BlockSize::_256bit, Groups::_1group>;
    template<size_t> using Blocked512 = BloomFilterParameter<Variant::Blocked, Vectorization::Horizontal,
            BlockSize::_512bit, Groups::_1group>;

    template<size_t> using Sectorized128 = BloomFilterParameter<Variant::Sectorized, Vectorization::Horizontal,
            BlockSize::_128bit, Groups::_1group>;
    template<size_t> using Sectorized256 = BloomFilterParameter<Variant::Sectorized, Vectorization::Horizontal,
            BlockSize::_256bit, Groups::_1group>;
    template<size_t> using Sectorized512 = BloomFilterParameter<Variant::Sectorized, Vectorization::Horizontal,
            BlockSize::_512bit, Groups::_1group>;

    template<size_t> using Grouped2 = BloomFilterParameter<Variant::Grouped, Vectorization::Horizontal,
            BlockSize::_512bit, Groups::_2groups>;
    template<size_t> using Grouped4 = BloomFilterParameter<Variant::Grouped, Vectorization::Horizontal,
            BlockSize::_512bit, Groups::_4groups>;
    template<size_t> using Grouped8 = BloomFilterParameter<Variant::Grouped, Vectorization::Horizontal,
            BlockSize::_512bit, Groups::_8groups>;

    template<size_t> using Sectorized128Vertical = BloomFilterParameter<Variant::Sectorized, Vectorization::Vertical,
            BlockSize::_128bit, Groups::_1group>;
    template<size_t> using Sectorized256Vertical = BloomFilterParameter<Variant::Sectorized, Vectorization::Vertical,
            BlockSize::_256bit, Groups::_1group>;
    template<size_t> using Sectorized512Vertical = BloomFilterParameter<Variant::Sectorized, Vectorization::Vertical,
            BlockSize::_512bit, Groups::_1group>;

} // filters::bloom
