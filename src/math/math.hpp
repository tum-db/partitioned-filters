#pragma once

#include <cstddef>
#include <cstdint>
#include <compiler/compiler_hints.hpp>
#include <assert.h>

namespace filters {
    namespace math {

        forceinline
        uint64_t consume_first_bits(uint64_t &val, size_t n_bits) {
            uint64_t ret = val & ((1ull << n_bits) - 1);
            val = val >> n_bits;
            return ret;
        }

        forceinline
        constexpr bool is_power_of_two(size_t x) {
            return (x == 1) | (x && ((x & (x - 1)) == 0));
        }

        template<typename T>
        forceinline
        constexpr T next_power_of_two(T value) {
            if (value == 0) {
                return 0;
            } else if (value == 1) {
                return 1;
            } else {
                if constexpr (sizeof(T) == sizeof(uint32_t)) {
                    return 1ull << ((sizeof(T) << 3) - __builtin_clz(value - 1));
                } else {
                    return 1ull << ((sizeof(T) << 3) - __builtin_clzll(value - 1));
                }
            }
        }

        template<typename T>
        forceinline
        constexpr T prev_power_of_two(T value) {
            return next_power_of_two(value) >> 1;
        }

        template<typename T>
        forceinline
        constexpr T get_number_of_bits(T value) {
            if (value == 0) {
                return 0;
            } else if (value == 1) {
                return 1;
            } else {
                if constexpr (sizeof(T) == sizeof(uint32_t)) {
                    return (sizeof(T) << 3) - __builtin_clz(value - 1);
                } else {
                    return (sizeof(T) << 3) - __builtin_clzll(value - 1);
                }
            }
        }

        template<typename T>
        forceinline
        constexpr T align_number(const T &value, const T &align) {
            return ((value + align - 1) / align) * align;
        }

        forceinline
        constexpr size_t const_log2(const size_t n) {
            return (n < 2)
                   ? 0
                   : 1 + const_log2(n / 2);
        }
    }
}