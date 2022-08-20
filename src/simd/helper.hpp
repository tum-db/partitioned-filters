#pragma once

#include <cstddef>
#include <cstring>
#include <compiler/compiler_hints.hpp>
#include <x86intrin.h>
#include <math/math.hpp>

#if defined(HAVE_NUMA)
#include <numa.h>
#endif

namespace filters::simd {

    template<typename Vector>
    forceinline
    static Vector extractBits(const Vector &vector, const size_t offset, const size_t n_bits) {
        return (vector >> (static_cast<size_t>(Vector::registerSize) - offset - n_bits)) & Vector((1ull << n_bits) - 1);
    }

    template<typename Vector>
    forceinline
    static Vector extractAddressBits(const Vector &vector) {
        return vector & Vector((1ull << 32) - 1);
    }

    forceinline
    static uint32_t pext32(uint32_t a, uint32_t mask) {
        #ifdef __BMI2__
        return _pext_u32(a, mask);
        #else
        uint32_t res = 0;
        size_t m = 0;
        size_t k = 0;
        while(m < 32){
            if((mask >> m) & 0x1){
                res |= ((a >> m) & 0x1) << k;
                k++;
            }
            m++;
        }
        return res;
        #endif
    }

    forceinline
    static uint64_t pext64(uint64_t a, uint64_t mask) {
        #ifdef __BMI2__
        return _pext_u64(a, mask);
        #else
        uint64_t res = 0;
        size_t m = 0;
        size_t k = 0;
        while(m < 64){
            if((mask >> m) & 0x1){
                res |= ((a >> m) & 0x1) << k;
                k++;
            }
            m++;
        }
        return res;
        #endif
    }

    forceinline
    static uint32_t tzcount32(uint32_t a) {
        #ifdef __BMI__
        return _tzcnt_u32(a);
        #else
        uint32_t ret = 0;
        while ((a & 0b1) == 0) {
            ret++;
            a = 0x80000000 | (a >> 1);
        }
        return ret;
        #endif
    }

    forceinline
    static uint64_t tzcount64(uint64_t a) {
        #ifdef __BMI__
        return _tzcnt_u64(a);
        #else
        uint64_t ret = 0;
        while ((a & 0b1) == 0) {
            ret++;
            a = 0x8000000000000000ull | (a >> 1);
        }
        return ret;
        #endif
    }

    template<typename T>
    static T tzcount(T a) {
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return tzcount64(a);
        } else {
            return tzcount32(a);
        }
    }

    template<typename Vector>
    forceinline
    static size_t valign(size_t n_bytes) {
        static constexpr size_t alignment = 64;
        if (n_bytes % alignment != 0) {
            n_bytes = (n_bytes / alignment + 1) * alignment;
        }
        return n_bytes;
    }

    template<typename Vector>
    forceinline
    static typename Vector::T *valloc(size_t n_bytes) {
        static constexpr size_t alignment = 64;
        return reinterpret_cast<typename Vector::T *>(aligned_alloc(alignment, n_bytes));
    }

    template<typename Vector>
    forceinline
    static typename Vector::T *valloc(size_t n_bytes, uint8_t val, bool numa = false) {
        static constexpr size_t alignment = 64;
        void *mem;
        #if defined(HAVE_NUMA)
        if (numa){
            mem = numa_alloc_interleaved(n_bytes);
        } else {
            mem = aligned_alloc(alignment, n_bytes);
        }
        #else
        mem = aligned_alloc(alignment, n_bytes);
        #endif
        std::memset(mem, val, n_bytes);
        return reinterpret_cast<typename Vector::T *>(mem);
    }

    forceinline
    static void vfree(void *mem, size_t n_bytes, bool numa = false) {
        #if defined(HAVE_NUMA)
        if (numa){
            numa_free(mem, n_bytes);
        } else {
            free(mem);
        }
        #else
        static_cast<void>(n_bytes);
        static_cast<void>(numa);
        free(mem);
        #endif
    }

    template<size_t n_bits, bool _nbit_aligned, typename Vector>
    forceinline
    static Vector _gatheru_bits(const Vector &bit_address, const uint8_t *base, const typename Vector::M &mask) {
        using T = typename Vector::T;
        const Vector value_mask = Vector((n_bits == sizeof(T) * 8)
                                         ? static_cast<T>(-1)
                                         : (1ull << n_bits) - 1);

        if constexpr (_nbit_aligned and math::is_power_of_two(n_bits)) {
            const Vector bucket_address = bit_address >> math::const_log2(sizeof(T) * 8);
            const Vector shift_right = bit_address & Vector((1ull << math::const_log2(sizeof(T) * 8)) - 1);
            return (bucket_address.gather(base, mask) >> shift_right) & value_mask;
        } else if constexpr (not Vector::avx and sizeof(T) * 8 >= 8 + n_bits) {
            const T byte_address = bit_address.vector / 8;
            const T shift_right = bit_address.vector % 8;
            return Vector(unaligned_load<T>(base + byte_address) >> shift_right) & value_mask;
        } else {
            const Vector bucket_address = bit_address >> math::const_log2(sizeof(T) * 8);
            const Vector shift_right = bit_address & Vector((1ull << math::const_log2(sizeof(T) * 8)) - 1);
            const Vector shift_left = Vector(sizeof(T) * 8 - 1) - shift_right;
            const Vector value1 = bucket_address.gather(base, mask) >> shift_right;
            // shift left can be 32/64
            const Vector value2 = ((bucket_address + Vector(1)).gather(base, mask) << 1) << shift_left;
            return (value1 | value2) & value_mask;
        }
    }

    template<size_t n_bits, typename Vector>
    forceinline
    static Vector gatheru_bits(const Vector &address, const uint8_t *base, const typename Vector::M &mask) {
        const Vector bit_address = address * Vector(n_bits);
        return _gatheru_bits<n_bits, true>(bit_address, base, mask);
    }

    template<size_t n_bits, typename Vector>
    forceinline
    static Vector gatheru_bits(const Vector &address, const uint8_t *base, const Vector &byte_offset,
                               const typename Vector::M &mask) {
        const Vector bit_address = address * Vector(n_bits) + (byte_offset << 3);
        return _gatheru_bits<n_bits, true>(bit_address, base, mask);
    }

    template<size_t n_bits, bool _nbit_aligned, typename Vector>
    forceinline
    static void
    _scatteru_bits(const Vector &bit_address, uint8_t *base, const Vector &value, const typename Vector::M &mask) {
        using T = typename Vector::T;
        const Vector value_mask = Vector((n_bits == sizeof(T) * 8)
                                         ? static_cast<T>(-1)
                                         : (1ull << n_bits) - 1);
        const Vector masked_value = value & value_mask;

        if constexpr (_nbit_aligned and math::is_power_of_two(n_bits)) {
            const Vector bucket_address = bit_address >> math::const_log2(sizeof(T) * 8);
            const Vector shift_left = bit_address & Vector((1ull << math::const_log2(sizeof(T) * 8)) - 1);
            const Vector old_value = bucket_address.gather(base, mask);
            const Vector merged_value =
                    (old_value & (value_mask << shift_left).negate()) | (masked_value << shift_left);
            merged_value.scatter(base, bucket_address, mask);
        } else if constexpr (not Vector::avx && sizeof(T) * 8 >= 8 + n_bits) {
            const T byte_address = bit_address.vector / 8;
            const T shift_left = bit_address.vector % 8;
            reinterpret_cast<unaligned<T> *>(base + byte_address)->value &= (value_mask << shift_left).negate().vector;
            reinterpret_cast<unaligned<T> *>(base + byte_address)->value |= (masked_value << shift_left).vector;
        } else {
            const Vector bucket_address = bit_address >> math::const_log2(sizeof(T) * 8);
            const Vector shift_left = bit_address & Vector((1ull << math::const_log2(sizeof(T) * 8)) - 1);
            const Vector shift_right = Vector(sizeof(T) * 8 - 1) - shift_left;

            const Vector old_value1 = bucket_address.gather(base, mask);
            const Vector merged_value1 =
                    (old_value1 & (value_mask << shift_left).negate()) | (masked_value << shift_left);
            merged_value1.scatter(base, bucket_address, mask);

            const Vector old_value2 = (bucket_address + Vector(1)).gather(base, mask);
            // shift right can be 32/64
            const Vector merged_value2 =
                    (old_value2 & ((value_mask >> 1) >> shift_right).negate()) | ((masked_value >> 1) >> shift_right);
            merged_value2.scatter(base, (bucket_address + Vector(1)), mask);
        }
    }

    template<size_t n_bits, typename Vector>
    forceinline
    static void
    scatteru_bits(const Vector &address, uint8_t *base, const Vector &value, const typename Vector::M &mask) {
        const Vector bit_address = address * Vector(n_bits);
        _scatteru_bits<n_bits, true>(bit_address, base, value, mask);
    }

    template<size_t n_bits, typename Vector>
    forceinline
    static void scatteru_bits(const Vector &address, uint8_t *base, const Vector &byte_offset, const Vector &value,
                              const typename Vector::M &mask) {
        const Vector bit_address = address * Vector(n_bits) + (byte_offset << 3);
        _scatteru_bits<n_bits, true>(bit_address, base, value, mask);
    }

    template<size_t k, size_t counter, typename T>
    forceinline unroll_loops
    constexpr T set_every_kth_bit() {
        if constexpr (counter == 0) {
            return 0;
        } else {
            return (set_every_kth_bit<k, counter - 1, T>() << k) | 0b1;
        }
    }

    template<size_t n_bits, size_t offset, typename Vector>
    forceinline unroll_loops
    static Vector sum_bits_in_registers(const Vector &x) {
        using T = typename Vector::T;
        constexpr T popcount_mask = set_every_kth_bit<n_bits, sizeof(T) * 8 / n_bits + 1, T>();

        Vector counter(0);
        for (size_t i = 0; i < n_bits; i++) {
            counter = counter + ((x & Vector(popcount_mask << i)).popcount() << ((i + offset) % n_bits));
        }
        return counter;
    }
}
