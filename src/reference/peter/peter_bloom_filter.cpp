#include "peter_bloom_filter.hpp"
#include <filter_base.hpp>

namespace filters
{
    template <parameter::HashingMode hashingMode>
    uint32_t containsMany(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf)
    {
        using Scalar = simd::Vector<parameter::RegisterSize::_64bit, parameter::SIMD::Scalar>;
        using T = typename Scalar::T;
        using Hasher = hash::Hasher<hashingMode, Scalar, 0>;

        static constexpr size_t LANES = 16;
        uint32_t count = 0;

        for (int j = 0; j + LANES <= num; j += LANES)
        {
            union
            {
                uint32_t key[2 * LANES];
                uint64_t hash[LANES];
            };

            for (int i = 0; i < LANES; i++)
            {
                hash[i] = Hasher::hash(Scalar(values[j + i])).vector;
            }

            uint32_t mask1[LANES], mask2[LANES], block1[LANES], block2[LANES];
            for (int i = 0; i < LANES; i++)
            {
                uint32_t key_hi = key[i + i + 1];
                uint32_t key_lo = key[i + i + 0];

                // key_lo |5:bit3|5:bit2|5:bit1|  13:block   |4:sector1 | bit layout (32:total)
                // key_hi |5:bit4|5:bit3|5:bit2|5:bit1|9:block|3:sector2| bit layout (32:total)
                block1[i] = ((key_lo & ((1 << 17) - 1)) + // key_lo 13:block|4:sector1
                        ((key_hi << 14) & (((1 << 9) - 1) << 17))) // key_hi 9:block
                    & (num_blocks - 1);

                mask1[i] = (1 << ((key_lo >> 17) & 31)) // key_lo 5:bit1
                    | (1 << ((key_lo >> 22) & 31)) // key_lo 5:bit2
                    | (1 << ((key_lo >> 27) & 31)); // key_lo 5:bit3
                block2[i] = block1[i] ^ (8 + (key_hi & 7)); // key_hi 3:sector2
                mask2[i] = (1 << ((key_hi >> 12) & 31)) // key_hi 5:bit1
                    | (1 << ((key_hi >> 17) & 31)) // key_hi 5:bit2
                    | (1 << ((key_hi >> 22) & 31)) // key_hi 5:bit3
                    | (1 << ((key_hi >> 27) & 31)); // key_hi 5:bit4
            }
            for (int i = 0; i < LANES; i++)
            {
                count += ((bf[block2[i]] & mask2[i]) == mask2[i]) & ((bf[block1[i]] & mask1[i]) == mask1[i]);
            }
        }

        for (int p = num & ~(LANES - 1); p < num; p++)
        {
            const uint64_t h = Hasher::hash(Scalar(values[p])).vector;
            uint32_t key_lo = static_cast<uint32_t>(h);
            uint32_t key_hi = h >> 32;
            uint32_t block1 = ((key_lo & ((1 << 17) - 1)) + // key_lo 13:block|4:sector1
                    ((key_hi << 14) & (((1 << 9) - 1) << 17))) // key_hi 9:block
                & (num_blocks - 1);
            uint32_t mask1 = (1 << ((key_lo >> 17) & 31)) // key_lo 5:bit1
                | (1 << ((key_lo >> 22) & 31)) // key_lo 5:bit2
                | (1 << ((key_lo >> 27) & 31)); // key_lo 5:bit3
            uint32_t block2 = block1 ^ (8 + (key_hi & 7)); // key_hi 3:sector2
            uint32_t mask2 = (1 << ((key_hi >> 12) & 31)) // key_hi 5:bit1
                | (1 << ((key_hi >> 17) & 31)) // key_hi 5:bit2
                | (1 << ((key_hi >> 22) & 31)) // key_hi 5:bit3
                | (1 << ((key_hi >> 27) & 31)); // key_hi 5:bit4
            count += ((bf[block2] & mask2) == mask2) & ((bf[block1] & mask1) == mask1);
        }
        return count;
    }

    template
    uint32_t containsMany<parameter::HashingMode::Identity>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t containsMany<parameter::HashingMode::Murmur>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t containsMany<parameter::HashingMode::Fasthash>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t containsMany<parameter::HashingMode::TwoIndependentMultiplyShift>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t containsMany<parameter::HashingMode::Mul>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
    template
    uint32_t containsMany<parameter::HashingMode::Cityhash>(int num, uint32_t num_blocks, const uint64_t* BF_RESTRICT values, const uint32_t* BF_RESTRICT bf);
}
