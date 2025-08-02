#include "umbra_bloom_filter.hpp"
#include <filter_base.hpp>

namespace filters
{
    template <parameter::HashingMode hashingMode>
    uint32_t umbraContainsMany(int num, uint32_t shift, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf)
    {
        using Scalar = simd::Vector<parameter::RegisterSize::_64bit, parameter::SIMD::Scalar>;
        using T = typename Scalar::T;
        using Hasher = hash::Hasher<hashingMode, Scalar, 0>;

        static constexpr size_t LANES = 16;
        uint32_t count = 0;
        shift = shift - 32;
        const uint32_t* BF_RESTRICT bm = reinterpret_cast<const uint32_t* BF_RESTRICT>(JoinFilter::bloomMasks);

        for (int j = 0; j + LANES <= num; j += LANES)
        {
            union
            {
                uint32_t key[2 * LANES];
                uint64_t hash[LANES];
            };

            for (int i = 0; i < LANES; i++)
            {
                hash[i] = Hasher::hash(Scalar(values[i + j])).vector;
            }

            bool found[LANES];
            for(int i = 0; i < LANES; i++){
                uint32_t b = key[i + i + 1] >> shift;
                uint32_t m = key[i + i + 0] >> 21;

                uint32_t block = (bf[b >> 1] >> ((b & 1) * 16)) & ((1 << 16) - 1);
                uint32_t mask = (bm[m >> 1] >> ((m & 1) * 16)) & ((1 << 16) - 1);

                found[i] = ((block & mask) == mask);
            }
            for (int i = 0; i < LANES; i++){
                count += found[i];
            }
        }

        for (int p = num & ~(LANES - 1); p < num; p++)
        {
            const uint64_t h = Hasher::hash(Scalar(values[p])).vector;
            uint32_t b = (h >> 32) >> shift;
            uint32_t m = static_cast<uint32_t>(h) >> 21;
            uint32_t block = (bf[b >> 1] >> ((b & 1) * 16)) & ((1 << 16) - 1);
            uint32_t mask = (bm[m >> 1] >> ((m & 1) * 16)) & ((1 << 16) - 1);
            bool found = (block & mask) == mask;
            count += (block & mask) == mask;
        }
        return count;
    }

    template <parameter::HashingMode hashingMode>
    uint32_t umbraContainsMany2(int num, uint32_t shift, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf)
    {
        using Scalar = simd::Vector<parameter::RegisterSize::_64bit, parameter::SIMD::Scalar>;
        using T = typename Scalar::T;
        using Hasher = hash::Hasher<hashingMode, Scalar, 0>;

        static constexpr size_t LANES = 8;
        uint32_t count = 0;
        const uint64_t* BF_RESTRICT bm = reinterpret_cast<const uint64_t* BF_RESTRICT>(JoinFilter::bloomMasks);

        for (int j = 0; j + LANES <= num; j += LANES)
        {
            for (int i = 0; i < LANES; i++)
            {
                uint64_t hash = Hasher::hash(Scalar(values[i + j])).vector;
                uint64_t b = hash >> shift;
                uint64_t m = (hash << 32) >> (32  + 21);

                uint64_t block = (bf[b >> 2] >> ((b & 3) * 16)) & ((1 << 16) - 1);
                uint64_t mask = (bm[m >> 2] >> ((m & 3) * 16)) & ((1 << 16) - 1);

                count += ((block & mask) == mask);
            }
        }

        for (int p = num & ~(LANES - 1); p < num; p++)
        {
            const uint64_t hash = Hasher::hash(Scalar(values[p])).vector;
            uint64_t b = hash >> shift;
            uint64_t m = (hash << 32) >> (32  + 21);

            uint64_t block = (bf[b >> 2] >> ((b & 3) * 16)) & ((1 << 16) - 1);
            uint64_t mask = (bm[m >> 2] >> ((m & 3) * 16)) & ((1 << 16) - 1);

            count += ((block & mask) == mask);
        }
        return count;
    }

    template
    uint32_t umbraContainsMany<parameter::HashingMode::Identity>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany<parameter::HashingMode::Murmur>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany<parameter::HashingMode::Fasthash>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany<parameter::HashingMode::TwoIndependentMultiplyShift>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany<parameter::HashingMode::Mul>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany<parameter::HashingMode::Cityhash>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);



    template
    uint32_t umbraContainsMany2<parameter::HashingMode::Identity>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany2<parameter::HashingMode::Murmur>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany2<parameter::HashingMode::Fasthash>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany2<parameter::HashingMode::TwoIndependentMultiplyShift>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany2<parameter::HashingMode::Mul>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);
    template
    uint32_t umbraContainsMany2<parameter::HashingMode::Cityhash>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint64_t* BF_RESTRICT bf);
}
