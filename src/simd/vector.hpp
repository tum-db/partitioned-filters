#pragma once

#include <cstdint>
#include <immintrin.h>
#include <bit>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>

namespace filters::simd {

    using RegisterSize = parameter::RegisterSize;
    using SIMD = parameter::SIMD;

    template<RegisterSize, SIMD>
    union Vector {
    };

    template<RegisterSize registerSize, SIMD simd>
    std::ostream &operator<<(std::ostream &os, const Vector<registerSize, simd> &vector) {
        os << std::string{"("} << vector.elements[0];
        for (size_t i = 1; i < Vector<registerSize, simd>::n_elements; i++) {
            os << std::string{", "} << vector.elements[i];
        }
        os << std::string{")"};
        return os;
    }

    template<RegisterSize _registerSize>
    union Vector<_registerSize, SIMD::Scalar> {
        using T = typename std::conditional<_registerSize == RegisterSize::_32bit, uint32_t, uint64_t>::type;
        using M = bool;
        using V = Vector<_registerSize, SIMD::Scalar>;

        static constexpr RegisterSize registerSize = _registerSize;
        static constexpr bool avx = false;
        static constexpr size_t n_elements = 1;

        T vector;
        T elements[n_elements] = {0};

        Vector() {
        }

        forceinline
        explicit Vector(const T val) : vector(val) {
        }

        forceinline
        V operator>>(const size_t n_bits) const {
            return V(vector >> n_bits);
        }

        forceinline
        V operator<<(const size_t n_bits) const {
            return V(vector << n_bits);
        }

        template<size_t n_bits>
        forceinline
        V ror() const {
            return Vector(std::rotr(vector, n_bits));
        }

        template<size_t n_bits>
        forceinline
        V rol() const {
            return Vector(std::rotl(vector, n_bits));
        }

        forceinline
        V operator>>(const V &v) const {
            return V(vector >> v.vector);
        }

        forceinline
        V operator<<(const V &v) const {
            return V(vector << v.vector);
        }

        forceinline
        V ror(const V &v) const {
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_rotr(vector, v.vector));
            } else {
                return V(_rotr64(vector, v.vector));
            }
        }

        forceinline
        V rol(const V &v) const {
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_rotl(vector, v.vector));
            } else {
                return V(_rotl64(vector, v.vector));
            }
        }

        forceinline
        V operator&(const V &v) const {
            return V(vector & v.vector);
        }

        forceinline
        V operator|(const V &v) const {
            return V(vector | v.vector);
        }

        forceinline
        V operator^(const V &v) const {
            return V(vector ^ v.vector);
        }

        forceinline
        V operator+(const V &v) const {
            return V(vector + v.vector);
        }

        forceinline
        V negate() const {
            return Vector(~vector);
        }

        forceinline
        V and_mask(const V &v, const M) const {
            return *this & v;
        }

        forceinline
        V or_mask(const V &v, const M) const {
            return *this | v;
        }

        forceinline
        V add_mask(const V &v, const M) const {
            return *this + v;
        }

        forceinline
        V operator-(const V &v) const {
            return V(vector - v.vector);
        }

        forceinline
        V operator*(const V &v) const {
            return V(vector * v.vector);
        }

        forceinline
        V permute(const V &) const {
            return *this;
        }

        forceinline
        V permute_regxbyte() const {
            V res(0);
            const uint8_t *src = reinterpret_cast<const uint8_t *>(&vector);
            uint8_t *dest = reinterpret_cast<uint8_t * >(&res.vector);
            for (size_t i = 0; i < sizeof(V); i++) {
                dest[i] = src[0];
            }
            return res;
        }

        forceinline
        V permute_regxword() const {
            V res(0);
            const uint16_t *src = reinterpret_cast<const uint16_t *>(&vector);
            uint16_t *dest = reinterpret_cast<uint16_t * >(&res.vector);
            for (size_t i = 0; i < sizeof(V) / sizeof(uint16_t); i++) {
                dest[i] = src[0];
            }
            return res;
        }

        forceinline
        V mulh32(const V &v) const {
            if constexpr (registerSize == RegisterSize::_32bit) {
                const uint64_t prod = static_cast<uint64_t>(vector) * static_cast<uint64_t>(v.vector);
                return V(static_cast<uint32_t>(prod >> 32));
            } else {
                const uint64_t low1 = vector & ((1ull << 32) - 1);
                const uint64_t low2 = v.vector & ((1ull << 32) - 1);
                return V((low1 * low2) >> 32);
            }
        }

        template<size_t bits>
        forceinline
        uint64_t equal(const V &v) const {
            uint64_t res = 0;
            for (size_t i = sizeof(T) / (bits / 8); i > 0; i--) {
                res = res << 1;
                T a = (vector >> ((i - 1) * bits)) & ((1 << bits) - 1);
                T b = (v.vector >> ((i - 1) * bits)) & ((1 << bits) - 1);
                res |= (a == b);
            }
            return res;
        }

        forceinline
        M operator==(const V &v) const {
            return vector == v.vector;
        }

        forceinline
        M operator!=(const V &v) const {
            return vector != v.vector;
        }

        forceinline
        M operator<(const V &v) const {
            return vector < v.vector;
        }

        forceinline
        M operator>(const V &v) const {
            return vector > v.vector;
        }

        forceinline
        V gather(const void *base_address, const M) const {
            return *(reinterpret_cast<const V *>(base_address) + vector);
        }

        forceinline
        void scatter(void *base_address, const V &index, const M) const {
            *(reinterpret_cast<T *>(base_address) + index.vector) = vector;
        }

        forceinline
        static V load(const void *address) {
            return *(reinterpret_cast<const V *>(address));
        }

        forceinline
        void store(void *address) {
            *reinterpret_cast<V *>(address) = vector;
        }

        template<size_t n_lanes>
        forceinline
        V gatherload(const void *base_address, const M m) const {
            static_assert(n_lanes == 1, "can only load a single value");
            return this->gather(base_address, m);
        }

        template<size_t n_lanes>
        forceinline
        void scatterstore_or(void *base_address, const V &index, const M m) const {
            static_assert(n_lanes == 1, "can only load a single value");
            (index.gather(base_address, m) | *this).scatter(base_address, index, m);
        }

        forceinline
        Vector popcount() {
            if constexpr (registerSize == RegisterSize::_32bit) {
                return Vector(_mm_popcnt_u32(vector));
            } else {
                return Vector(_mm_popcnt_u64(vector));
            }
        }

        forceinline
        static V loadu(const void *address, const M) {
            return *(reinterpret_cast<const V *>(address));
        }

        forceinline
        static M mask(bool init) {
            return init;
        }

        forceinline
        static T popcount_mask(M mask) {
            return mask;
        }

    };

    template<RegisterSize _registerSize>
    union Vector<_registerSize, SIMD::AVX2> {
        using T = typename std::conditional<_registerSize == RegisterSize::_32bit, uint32_t, uint64_t>::type;
        using M = __mmask8;
        using V = Vector<_registerSize, SIMD::AVX2>;
        using SV = Vector<_registerSize, SIMD::Scalar>;

        static constexpr RegisterSize registerSize = _registerSize;
        static constexpr bool avx = true;
        static constexpr size_t n_sv_elements = static_cast<size_t>(SIMD::AVX2) / 8 / sizeof(SV);
        static constexpr size_t n_elements = static_cast<size_t>(SIMD::AVX2) / static_cast<size_t>(registerSize);

        __m256i vector;
        SV sv_elements[n_sv_elements];
        T elements[n_elements] = {0};

        Vector() {
        }

        forceinline
        explicit Vector(const T val) {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                vector = _mm256_set1_epi32(val);
            } else {
                vector = _mm256_set1_epi64x(val);
            }
            #else
            for(size_t i = 0; i < n_sv_elements; i++){
                sv_elements[i] = SV(val);
            }
            #endif
        }

        forceinline
        explicit Vector(const __m256i &vector) : vector(vector) {
        }

        forceinline
        V operator>>(const size_t n_bits) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_srli_epi32(vector, n_bits));
            } else {
                return V(_mm256_srli_epi64(vector, n_bits));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = (n_bits < sizeof(T) * 8)
                                     ? sv_elements[i] >> n_bits
                                     : SV(0);
            }
            return res;
            #endif
        }

        forceinline
        V operator<<(const size_t n_bits) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_slli_epi32(vector, n_bits));
            } else {
                return V(_mm256_slli_epi64(vector, n_bits));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = (n_bits < sizeof(T) * 8)
                                     ? sv_elements[i] << n_bits
                                     : SV(0);
            }
            return res;
            #endif
        }

        template<size_t n_bits>
        forceinline
        V ror() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_ror_epi32(vector, n_bits));
            } else {
                return V(_mm256_ror_epi64(vector, n_bits));
            }
            #else
            #ifdef __AVX2__
            const size_t mask = static_cast<size_t>(registerSize) - 1;
            return (*this >> (n_bits & mask)) | (*this << (static_cast<size_t>(registerSize) - (n_bits & mask)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].template ror<n_bits>();
            }
            return res;
            #endif
            #endif
        }

        template<size_t n_bits>
        forceinline
        V rol() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_rol_epi32(vector, n_bits));
            } else {
                return V(_mm256_rol_epi64(vector, n_bits));
            }
            #else
            #ifdef __AVX2__
            const size_t mask = static_cast<size_t>(registerSize) - 1;
            return (*this << (n_bits & mask)) | (*this >> (static_cast<size_t>(registerSize) - (n_bits & mask)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].template rol<n_bits>();
            }
            return res;
            #endif
            #endif
        }

        forceinline
        V operator>>(const V &v) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_srlv_epi32(vector, v.vector));
            } else {
                return V(_mm256_srlv_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = (v.sv_elements[i].vector < sizeof(T) * 8)
                                     ? sv_elements[i] >> v.sv_elements[i]
                                     : SV(0);
            }
            return res;
            #endif
        }

        forceinline
        V operator<<(const V &v) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_sllv_epi32(vector, v.vector));
            } else {
                return V(_mm256_sllv_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = (v.sv_elements[i].vector < sizeof(T) * 8)
                                     ? sv_elements[i] << v.sv_elements[i]
                                     : SV(0);
            }
            return res;
            #endif
        }

        forceinline
        V ror(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_rorv_epi32(vector, v.vector));
            } else {
                return V(_mm256_rorv_epi64(vector, v.vector));
            }
            #else
            #ifdef __AVX2__
            const V mask = V(static_cast<size_t>(registerSize) - 1);
            return (*this >> (v & mask)) | (*this << (V(static_cast<size_t>(registerSize)) - (v & mask)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].ror(v.sv_elements[i]);
            }
            return res;
            #endif
            #endif
        }

        forceinline
        V rol(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_rolv_epi32(vector, v.vector));
            } else {
                return V(_mm256_rolv_epi64(vector, v.vector));
            }
            #else
            #ifdef __AVX2__
            const V mask = V(static_cast<size_t>(registerSize) - 1);
            return (*this << (v & mask)) | (*this >> (V(static_cast<size_t>(registerSize)) - (v & mask)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].rol(v.sv_elements[i]);
            }
            return res;
            #endif
            #endif
        }

        forceinline
        V operator&(const V &v) const {
            #ifdef __AVX2__
            return V(_mm256_and_si256(vector, v.vector));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] & v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator|(const V &v) const {
            #ifdef __AVX2__
            return V(_mm256_or_si256(vector, v.vector));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] | v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator^(const V &v) const {
            #ifdef __AVX2__
            return V(_mm256_xor_si256(vector, v.vector));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] ^ v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator+(const V &v) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_add_epi32(vector, v.vector));
            } else {
                return V(_mm256_add_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] + v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V negate() const {
            #ifdef __AVX2__
            return V(_mm256_xor_si256(vector, _mm256_set1_epi8(0xFF)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].negate();
            }
            return res;
            #endif
        }

        forceinline
        V and_mask(const V &v, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_mask_and_epi32(vector, mask, vector, v.vector));
            } else {
                return V(_mm256_mask_and_epi64(vector, mask, vector, v.vector));
            }
            #else
            V res(vector);
            for (size_t i = 0; i < n_sv_elements; i++) {
                if (mask >> i & 0b1) {
                    res.sv_elements[i] = sv_elements[i] & v.sv_elements[i];
                }
            }
            return res;
            #endif
        }

        forceinline
        V or_mask(const V &v, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_mask_or_epi32(vector, mask, vector, v.vector));
            } else {
                return V(_mm256_mask_or_epi64(vector, mask, vector, v.vector));
            }
            #else
            V res(vector);
            for (size_t i = 0; i < n_sv_elements; i++) {
                if (mask >> i & 0b1) {
                    res.sv_elements[i] = sv_elements[i] | v.sv_elements[i];
                }
            }
            return res;
            #endif
        }

        forceinline
        V add_mask(const V &v, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_mask_add_epi32(vector, mask, vector, v.vector));
            } else {
                return V(_mm256_mask_add_epi64(vector, mask, vector, v.vector));
            }
            #else
            V res(vector);
            for (size_t i = 0; i < n_sv_elements; i++) {
                if (mask >> i & 0b1) {
                    res.sv_elements[i] = sv_elements[i] + v.sv_elements[i];
                }
            }
            return res;
            #endif
        }

        forceinline
        V operator-(const V &v) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_sub_epi32(vector, v.vector));
            } else {
                return V(_mm256_sub_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] - v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator*(const V &v) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_mullo_epi32(vector, v.vector));
            } else {
                #ifdef __AVX512F__
                return V(_mm256_mullo_epi64(vector, v.vector));
                #else
                // swap H<->L
                __m256i swap = _mm256_shuffle_epi32(vector, 0xB1);
                // 32 bit L*H products
                __m256i prodlh = _mm256_mullo_epi32(v.vector, swap);

                // 0, 0H*0L, 0, 1H*1L
                __m256i prodlh2 = _mm256_slli_epi64(prodlh, 32);
                // xxx, 0L*0H+0H*0L, xxx, 1L*1H+1H*1L
                __m256i prodlh3 = _mm256_add_epi32(prodlh2, prodlh);
                // 0, 0L*0H+0H*0L, 0, 1L*1H+1H*1L
                __m256i prodlh4 = _mm256_and_si256(prodlh3, _mm256_set1_epi64x(0xFFFFFFFF00000000));

                // 0L*0L,1L*1L, 64 bit unsigned products
                __m256i prodll = _mm256_mul_epu32(vector, v.vector);
                // 0L*0L+(0L*0H+0H*0L)<<32, 1L*1L+(1L*1H+1H*1L)<<32
                __m256i prod = _mm256_add_epi64(prodll, prodlh4);
                return V(prod);
                #endif
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] * v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V permute(const V &index) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_permutex2var_epi32(vector, index.vector, vector));
            } else {
                return V(_mm256_permutex2var_epi64(vector, index.vector, vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_elements; i++) {
                res.elements[i] = elements[index.elements[i] % n_elements];
            }
            return res;
            #endif
        }

        forceinline
        V permute_regxbyte() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m256i b = _mm256_set_epi8(12, 12, 12, 12, 8, 8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0, 12, 12, 12, 12, 8,
                        8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0);
                return V(_mm256_shuffle_epi8(vector, b));
            } else {
                const __m256i b = _mm256_set_epi8(8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8,
                        8, 0, 0, 0, 0, 0, 0, 0, 0);
                return V(_mm256_shuffle_epi8(vector, b));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].permute_regxbyte();
            }
            return res;
            #endif
        }

        forceinline
        V permute_regxword() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m256i b = _mm256_set_epi8(13, 12, 13, 12, 9, 8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0, 13, 12, 13, 12, 9,
                        8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0);
                return V(_mm256_shuffle_epi8(vector, b));
            } else {
                const __m256i b = _mm256_set_epi8(9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0, 9, 8, 9, 8, 9, 8, 9,
                        8, 1, 0, 1, 0, 1, 0, 1, 0);
                return V(_mm256_shuffle_epi8(vector, b));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].permute_regxword();
            }
            return res;
            #endif
        }

        forceinline
        V mulh32(const V &v) const {
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                // shift 32-bit input to 64-bit lanes
                const __m256i a_odd_u64 = _mm256_srli_epi64(vector, 32);
                const __m256i a_even_u64 = vector;
                const __m256i b_odd_u64 = _mm256_srli_epi64(v.vector, 32);
                const __m256i b_even_u64 = v.vector;
                // multiply
                const __m256i p_odd_u64 = _mm256_mul_epu32(a_odd_u64, b_odd_u64);
                const __m256i p_even_u64 = _mm256_mul_epu32(a_even_u64, b_even_u64);
                // merge the higher 32-bits of products back into a single YMM register
                const __m256i p_even_hi_u64 = _mm256_srli_epi64(p_even_u64, 32);
                return V(_mm256_blend_epi32(p_odd_u64, p_even_hi_u64, 0b01010101));
            } else {
                const V mask = V(0xFFFFFFFF);
                const V low1 = *this & mask;
                const V low2 = v & mask;
                return (low1 * low2) >> 32;
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].mulh32(v.sv_elements[i]);
            }
            return res;
            #endif
        }

        template<size_t bits>
        forceinline
        static uint32_t avx2MaskToAvx512Mask(const __m256i mask) {
            const uint32_t imask = _mm256_movemask_epi8(mask);
            if constexpr (bits == 8) {
                return imask;
            } else if constexpr (bits == 16) {
                return _pext_u32(imask, 0x55555555);
            } else if constexpr (bits == 32) {
                return _pext_u32(imask, 0x11111111);
            } else {
                return _pext_u32(imask, 0x01010101);
            }
        }

        template<size_t bits>
        forceinline
        uint64_t equal(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (bits == 8) {
                return _mm256_cmpeq_epi8_mask(vector, v.vector);
            } else if constexpr (bits == 16) {
                return _mm256_cmpeq_epi16_mask(vector, v.vector);
            } else if constexpr (bits == 32) {
                return _mm256_cmpeq_epi32_mask(vector, v.vector);
            } else {
                return _mm256_cmpeq_epi64_mask(vector, v.vector);
            }
            #else
            #ifdef __AVX2__
            __m256i mask;
            if constexpr (bits == 8) {
                mask = _mm256_cmpeq_epi8(vector, v.vector);
            } else if constexpr (bits == 16) {
                mask = _mm256_cmpeq_epi16(vector, v.vector);
            } else if constexpr (bits == 32) {
                mask = _mm256_cmpeq_epi32(vector, v.vector);
            } else {
                mask = _mm256_cmpeq_epi64(vector, v.vector);
            }
            return avx2MaskToAvx512Mask<bits>(mask);
            #else
            uint64_t res = 0;
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << (sizeof(SV) / (bits / 8));
                res |= sv_elements[i - 1].template equal<bits>(v.sv_elements[i - 1]);
            }
            return res;
            #endif
            #endif
        }

        forceinline
        M operator==(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm256_cmpeq_epi32_mask(vector, v.vector);
            } else {
                return _mm256_cmpeq_epi64_mask(vector, v.vector);
            }
            #else
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m256i mask = _mm256_cmpeq_epi32(vector, v.vector);
                return avx2MaskToAvx512Mask<32>(mask);
            } else {
                const __m256i mask = _mm256_cmpeq_epi64(vector, v.vector);
                return avx2MaskToAvx512Mask<64>(mask);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << 1;
                res |= (sv_elements[i - 1] == v.sv_elements[i - 1]);
            }
            return res;
            #endif
            #endif
        }

        forceinline
        M operator!=(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm256_cmpneq_epi32_mask(vector, v.vector);
            } else {
                return _mm256_cmpneq_epi64_mask(vector, v.vector);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << 1;
                res |= (sv_elements[i - 1] != v.sv_elements[i - 1]);
            }
            return res;
            #endif
        }

        forceinline
        M operator<(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm256_cmplt_epu32_mask(vector, v.vector);
            } else {
                return _mm256_cmplt_epu64_mask(vector, v.vector);
            }
            #else
            #ifdef __AVX2__
            // emulate unsigned compare by converting the vector to signed values
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m256i a = _mm256_xor_si256(_mm256_set1_epi32(0x80000000), v.vector);
                const __m256i b = _mm256_xor_si256(_mm256_set1_epi32(0x80000000), vector);
                const __m256i mask = _mm256_cmpgt_epi32(a, b);
                return avx2MaskToAvx512Mask<32>(mask);
            } else {
                const __m256i a = _mm256_xor_si256(_mm256_set1_epi64x(0x8000000000000000), v.vector);
                const __m256i b = _mm256_xor_si256(_mm256_set1_epi64x(0x8000000000000000), vector);
                const __m256i mask = _mm256_cmpgt_epi64(a, b);
                return avx2MaskToAvx512Mask<64>(mask);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << 1;
                res |= (sv_elements[i - 1] < v.sv_elements[i - 1]);
            }
            return res;
            #endif
            #endif
        }

        forceinline
        M operator>(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm256_cmpgt_epu32_mask(vector, v.vector);
            } else {
                return _mm256_cmpgt_epu64_mask(vector, v.vector);
            }
            #else
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m256i a = _mm256_xor_si256(_mm256_set1_epi32(0x80000000), vector);
                const __m256i b = _mm256_xor_si256(_mm256_set1_epi32(0x80000000), v.vector);
                const __m256i mask = _mm256_cmpgt_epi32(a, b);
                return avx2MaskToAvx512Mask<32>(mask);
            } else {
                const __m256i a = _mm256_xor_si256(_mm256_set1_epi64x(0x8000000000000000), vector);
                const __m256i b = _mm256_xor_si256(_mm256_set1_epi64x(0x8000000000000000), v.vector);
                const __m256i mask = _mm256_cmpgt_epi64(a, b);
                return avx2MaskToAvx512Mask<64>(mask);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << 1;
                res |= (sv_elements[i - 1] > v.sv_elements[i - 1]);
            }
            return res;
            #endif
            #endif
        }

        forceinline
        V gather(const void *base_address, const M mask) const {
            V res(0);
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_mmask_i32gather_epi32(res.vector, mask, vector, base_address, sizeof(T)));
            } else {
                return V(_mm256_mmask_i64gather_epi64(res.vector, mask, vector, base_address, sizeof(T)));
            }
            #else
            #ifdef __AVX2__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m256i shift = _mm256_set_epi32(24, 25, 26, 27, 28, 29, 30, 31);

                __m256i mmask = _mm256_set1_epi32(mask);
                mmask = _mm256_sllv_epi32(mmask, shift);
                mmask = _mm256_srai_epi32(mmask, 31);

                return V(_mm256_mask_i32gather_epi32(res.vector, reinterpret_cast<const int *>(base_address), vector,
                        mmask, sizeof(T)));
            } else {
                const __m256i shift = _mm256_set_epi32(28, 28, 29, 29, 30, 30, 31, 31);

                __m256i mmask = _mm256_set1_epi32(mask);
                mmask = _mm256_sllv_epi32(mmask, shift);
                mmask = _mm256_srai_epi32(mmask, 31);

                return V(_mm256_mask_i64gather_epi64(res.vector, reinterpret_cast<const long long int *>(base_address),
                        vector, mmask, sizeof(T)));
            }
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                if (mask >> i & 0b1) {
                    res.sv_elements[i] = sv_elements[i].gather(base_address, 1);
                }
            }
            return res;
            #endif
            #endif
        }

        forceinline
        void scatter(void *base_address, const V &index, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                _mm256_mask_i32scatter_epi32(base_address, mask, index.vector, vector, sizeof(T));
            } else {
                _mm256_mask_i64scatter_epi64(base_address, mask, index.vector, vector, sizeof(T));
            }
            #else
            // emulate scatter
            for (size_t i = 0; i < n_sv_elements; i++) {
                if (mask >> i & 0b1) {
                    sv_elements[i].scatter(base_address, index.sv_elements[i], 1);
                }
            }
            #endif
        }

        forceinline
        static V load(const void *address) {
            #ifdef __AVX2__
            return V(_mm256_load_si256(reinterpret_cast<const __m256i *>(address)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = SV::load(reinterpret_cast<const uint8_t *>(address) + sizeof(SV) * i);
            }
            return res;
            #endif
        }

        forceinline
        V store(void *address) {
            #ifdef __AVX2__
            _mm256_store_si256(reinterpret_cast<__m256i *>(address), vector);
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                SV::store(reinterpret_cast<const uint8_t *>(address) + sizeof(SV) * i);
            }
            #endif
        }

        template<size_t n_lanes>
        forceinline
        V gatherload(const void *base_address, const M mask) const {
            static_assert(n_lanes == 1 or n_lanes == 2 or n_lanes == 4 or
                          (n_lanes == 8 and registerSize == RegisterSize::_32bit), "unsupported number of lanes!");

            #ifdef __AVX512F__
            const V zero(0);
            if constexpr (n_lanes == 1 && registerSize == RegisterSize::_64bit) {
                return V(_mm256_mask_load_epi64(zero.vector, mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]));
            } else if constexpr (n_lanes == 1 && registerSize == RegisterSize::_32bit) {
                return V(_mm256_mask_load_epi32(zero.vector, mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]));
            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_64bit) {
                const __m128i lower = _mm_mask_load_epi64(_mm_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]);
                const __m128i upper = _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 2,
                        reinterpret_cast<const T *>(base_address) + elements[2]);
                return V(_mm256_inserti128_si256(_mm256_inserti128_si256(zero.vector, lower, 0), upper, 1));
            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_32bit) {
                const __m128i lower = _mm_mask_load_epi32(_mm_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]);
                const __m128i upper = _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 4,
                        reinterpret_cast<const T *>(base_address) + elements[4]);
                return V(_mm256_inserti128_si256(_mm256_inserti128_si256(zero.vector, lower, 0), upper, 1));
            } else {
                return this->gather(base_address, mask);
            }
            #else
            return this->gather(base_address, mask);
            #endif
        }

        template<size_t n_lanes>
        forceinline
        void scatterstore_or(void *base_address, const V &index, const M mask) const {
            static_assert(n_lanes == 1 or n_lanes == 2 or n_lanes == 4 or
                          (n_lanes == 8 and registerSize == RegisterSize::_32bit), "unsupported number of lanes!");

            #ifdef __AVX512F__
            if constexpr (n_lanes == 1 && registerSize == RegisterSize::_64bit) {
                const __m256i value = _mm256_or_si256(vector, _mm256_mask_load_epi64(_mm256_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm256_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value);

            } else if constexpr (n_lanes == 1 && registerSize == RegisterSize::_32bit) {
                const __m256i value = _mm256_or_si256(vector, _mm256_mask_load_epi32(_mm256_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm256_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value);

            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_64bit) {
                const __m128i value_lower = _mm_or_si128(_mm256_extracti128_si256(vector, 0),
                        _mm_mask_load_epi64(_mm_set1_epi32(0), mask,
                                reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value_lower);

                const __m128i value_upper = _mm_or_si128(_mm256_extracti128_si256(vector, 1),
                        _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 2,
                                reinterpret_cast<const T *>(base_address) + index.elements[2]));
                _mm_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[2], mask >> 2, value_upper);

            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_32bit) {
                const __m128i value_lower = _mm_or_si128(_mm256_extracti128_si256(vector, 0),
                        _mm_mask_load_epi32(_mm_set1_epi32(0), mask,
                                reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value_lower);

                const __m128i value_upper = _mm_or_si128(_mm256_extracti128_si256(vector, 1),
                        _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 4,
                                reinterpret_cast<const T *>(base_address) + index.elements[4]));
                _mm_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[4], mask >> 4, value_upper);

            } else {
                for (size_t i = 0; i < n_sv_elements; i++) {
                    if (mask >> i & 0b1) {
                        sv_elements[i].template scatterstore_or<1>(base_address, index.sv_elements[i], 1);
                    }
                }
            }
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                if (mask >> i & 0b1) {
                    sv_elements[i].template scatterstore_or<1>(base_address, index.sv_elements[i], 1);
                }
            }
            #endif
        }

        forceinline
        Vector popcount() {
            #ifdef __AVX2__
            const __m256i lookup = _mm256_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0, 4, 3, 3, 2, 3, 2, 2,
                    1, 3, 2, 2, 1, 2, 1, 1, 0);
            const __m256i low_mask = _mm256_set1_epi8(0x0f);
            const __m256i lo = _mm256_and_si256(vector, low_mask);
            const __m256i hi = _mm256_and_si256(_mm256_srli_epi16(vector, 4), low_mask);
            const __m256i cnt1 = _mm256_shuffle_epi8(lookup, lo);
            const __m256i cnt2 = _mm256_shuffle_epi8(lookup, hi);
            const __m256i cnt = _mm256_add_epi8(cnt1, cnt2);
            if constexpr (registerSize == RegisterSize::_64bit) {
                return Vector(_mm256_sad_epu8(cnt, _mm256_setzero_si256()));
            } else {
                const Vector i1 = Vector(cnt);
                const Vector i2 = i1 + (i1 >> 8);
                return (i2 + (i2 >> 16)) & Vector(0xFF);
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].popcount();
            }
            return res;
            #endif
        }

        forceinline
        static V loadu(const void *address, const M mask) {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_mask_loadu_epi32(_mm256_set1_epi32(0), mask, address));
            } else {
                return V(_mm256_mask_loadu_epi64(_mm256_set1_epi64x(0), mask, address));
            }
            #else
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0)).gather(address, mask);
            } else {
                return V(_mm256_set_epi64x(3, 2, 1, 0)).gather(address, mask);
            }
            #endif
        }

        forceinline
        static M mask(bool init) {
            V res(0);
            if constexpr (registerSize == RegisterSize::_32bit) {
                return 0xFF * init;
            } else {
                return 0xF * init;
            }
        }

        forceinline
        static T popcount_mask(M mask) {
            return _mm_popcnt_u32(mask);
        }
    };

    template<RegisterSize _registerSize>
    union Vector<_registerSize, SIMD::AVX512> {
        using T = typename std::conditional<_registerSize == RegisterSize::_32bit, uint32_t, uint64_t>::type;
        using M = typename std::conditional<_registerSize == RegisterSize::_32bit, __mmask16, __mmask8>::type;
        using V = Vector<_registerSize, SIMD::AVX512>;
        using SV = Vector<_registerSize, SIMD::AVX2>;

        static constexpr RegisterSize registerSize = _registerSize;
        static constexpr bool avx = true;
        static constexpr size_t n_sv_elements = static_cast<size_t>(SIMD::AVX512) / 8 / sizeof(SV);
        static constexpr size_t n_elements = static_cast<size_t>(SIMD::AVX512) / static_cast<size_t>(registerSize);

        __m512i vector;
        SV sv_elements[n_sv_elements];
        T elements[n_elements] = {0};

        Vector() {
        }

        forceinline
        explicit Vector(const T val) {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                vector = _mm512_set1_epi32(val);
            } else {
                vector = _mm512_set1_epi64(val);
            }
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                sv_elements[i] = SV(val);
            }
            #endif
        }

        forceinline
        explicit Vector(const __m512i &vector) : vector(vector) {
        }

        forceinline
        V operator>>(const size_t n_bits) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_srli_epi32(vector, n_bits));
            } else {
                return V(_mm512_srli_epi64(vector, n_bits));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] >> n_bits;
            }
            return res;
            #endif
        }

        forceinline
        V operator<<(const size_t n_bits) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_slli_epi32(vector, n_bits));
            } else {
                return V(_mm512_slli_epi64(vector, n_bits));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] << n_bits;
            }
            return res;
            #endif
        }

        template<size_t n_bits>
        forceinline
        V ror() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_ror_epi32(vector, n_bits));
            } else {
                return V(_mm512_ror_epi64(vector, n_bits));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].template ror<n_bits>();
            }
            return res;
            #endif
        }

        template<size_t n_bits>
        forceinline
        V rol() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_rol_epi32(vector, n_bits));
            } else {
                return V(_mm512_rol_epi64(vector, n_bits));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].template rol<n_bits>();
            }
            return res;
            #endif
        }

        forceinline
        V operator>>(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_srlv_epi32(vector, v.vector));
            } else {
                return V(_mm512_srlv_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] >> v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator<<(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_sllv_epi32(vector, v.vector));
            } else {
                return V(_mm512_sllv_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] << v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V ror(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_rorv_epi32(vector, v.vector));
            } else {
                return V(_mm512_rorv_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].ror(v.sv_elements[i]);
            }
            return res;
            #endif
        }

        forceinline
        V rol(const Vector &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_rolv_epi32(vector, v.vector));
            } else {
                return V(_mm512_rolv_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].rol(v.sv_elements[i]);
            }
            return res;
            #endif
        }

        forceinline
        V operator&(const V &v) const {
            #ifdef __AVX512F__
            return V(_mm512_and_si512(vector, v.vector));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] & v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator|(const V &v) const {
            #ifdef __AVX512F__
            return V(_mm512_or_si512(vector, v.vector));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] | v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator^(const V &v) const {
            #ifdef __AVX512F__
            return V(_mm512_xor_si512(vector, v.vector));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] ^ v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator+(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_add_epi32(vector, v.vector));
            } else {
                return V(_mm512_add_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] + v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V negate() const {
            #ifdef __AVX512F__
            return V(_mm512_xor_si512(vector, _mm512_set1_epi8(0xFF)));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].negate();
            }
            return res;
            #endif
        }

        forceinline
        V and_mask(const V &v, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_mask_and_epi32(vector, mask, vector, v.vector));
            } else {
                return V(_mm512_mask_and_epi64(vector, mask, vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].and_mask(v.sv_elements[i], mask >> (i * sizeof(SV) / sizeof(T)));
            }
            return res;
            #endif
        }

        forceinline
        V or_mask(const V &v, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_mask_or_epi32(vector, mask, vector, v.vector));
            } else {
                return V(_mm512_mask_or_epi64(vector, mask, vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].or_mask(v.sv_elements[i], mask >> (i * sizeof(SV) / sizeof(T)));
            }
            return res;
            #endif
        }

        forceinline
        V add_mask(const V &v, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_mask_add_epi32(vector, mask, vector, v.vector));
            } else {
                return V(_mm512_mask_add_epi64(vector, mask, vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].add_mask(v.sv_elements[i], mask >> (i * sizeof(SV) / sizeof(T)));
            }
            return res;
            #endif
        }

        forceinline
        V operator-(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_sub_epi32(vector, v.vector));
            } else {
                return V(_mm512_sub_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] - v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V operator*(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_mullo_epi32(vector, v.vector));
            } else {
                return V(_mm512_mullo_epi64(vector, v.vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i] * v.sv_elements[i];
            }
            return res;
            #endif
        }

        forceinline
        V permute(const V &index) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_permutex2var_epi32(vector, index.vector, vector));
            } else {
                return V(_mm512_permutex2var_epi64(vector, index.vector, vector));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_elements; i++) {
                res.elements[i] = elements[index.elements[i] % n_elements];
            }
            return res;
            #endif
        }

        forceinline
        V permute_regxbyte() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m512i b = _mm512_set_epi8(12, 12, 12, 12, 8, 8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0, 12, 12, 12, 12, 8,
                        8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0, 12, 12, 12, 12, 8, 8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0, 12, 12, 12, 12, 8,
                        8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0);
                return V(_mm512_shuffle_epi8(vector, b));
            } else {
                const __m512i b = _mm512_set_epi8(8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8,
                        8, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8,
                        8, 0, 0, 0, 0, 0, 0, 0, 0);
                return V(_mm512_shuffle_epi8(vector, b));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].permute_regxbyte();
            }
            return res;
            #endif
        }

        forceinline
        V permute_regxword() const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                const __m512i b = _mm512_set_epi8(13, 12, 13, 12, 9, 8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0, 13, 12, 13, 12, 9,
                        8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0, 13, 12, 13, 12, 9, 8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0, 13, 12, 13, 12, 9,
                        8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0);
                return V(_mm512_shuffle_epi8(vector, b));
            } else {
                const __m512i b = _mm512_set_epi8(9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0, 9, 8, 9, 8, 9, 8, 9,
                        8, 1, 0, 1, 0, 1, 0, 1, 0, 9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0, 9, 8, 9, 8, 9, 8, 9,
                        8, 1, 0, 1, 0, 1, 0, 1, 0);
                return V(_mm512_shuffle_epi8(vector, b));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].permute_regxword();
            }
            return res;
            #endif
        }

        forceinline
        V mulh32(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                // shift 32-bit input to 64-bit lanes
                const __m512i a_odd_u64 = _mm512_srli_epi64(vector, 32);
                const __m512i a_even_u64 = vector;
                const __m512i b_odd_u64 = _mm512_srli_epi64(v.vector, 32);
                const __m512i b_even_u64 = v.vector;
                // multiply
                const __m512i p_odd_u64 = _mm512_mul_epu32(a_odd_u64, b_odd_u64);
                const __m512i p_even_u64 = _mm512_mul_epu32(a_even_u64, b_even_u64);
                // merge the higher 32-bits of products back into a single ZMM register
                const __m512i p_even_hi_u64 = _mm512_srli_epi64(p_even_u64, 32);
                return V(_mm512_mask_mov_epi32(p_odd_u64, __mmask16(0b0101010101010101), p_even_hi_u64));
            } else {
                const V mask = V((static_cast<T>(1) << 32) - 1);
                const V low1 = *this & mask;
                const V low2 = v & mask;
                return (low1 * low2) >> 32;
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].mulh32(v.sv_elements[i]);
            }
            return res;
            #endif
        }

        template<size_t bits>
        forceinline
        uint64_t equal(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (bits == 8) {
                return _mm512_cmpeq_epi8_mask(vector, v.vector);
            } else if constexpr (bits == 16) {
                return _mm512_cmpeq_epi16_mask(vector, v.vector);
            } else if constexpr (bits == 32) {
                return _mm512_cmpeq_epi32_mask(vector, v.vector);
            } else {
                return _mm512_cmpeq_epi64_mask(vector, v.vector);
            }
            #else
            uint64_t res = 0;
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << (sizeof(SV) / (bits / 8));
                res |= sv_elements[i - 1].template equal<bits>(v.sv_elements[i - 1]);
            }
            return res;
            #endif
        }

        forceinline
        M operator==(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm512_cmpeq_epi32_mask(vector, v.vector);
            } else {
                return _mm512_cmpeq_epi64_mask(vector, v.vector);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << (sizeof(SV) / sizeof(T));
                res |= sv_elements[i - 1] == v.sv_elements[i - 1];
            }
            return res;
            #endif
        }

        forceinline
        M operator!=(const V &v) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm512_cmpneq_epi32_mask(vector, v.vector);
            } else {
                return _mm512_cmpneq_epi64_mask(vector, v.vector);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << (sizeof(SV) / sizeof(T));
                res |= sv_elements[i - 1] != v.sv_elements[i - 1];
            }
            return res;
            #endif
        }

        forceinline
        M operator<(const V &v) const {
            #ifdef __AVX512F__
            V res(0);
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm512_cmplt_epu32_mask(vector, v.vector);
            } else {
                return _mm512_cmplt_epu64_mask(vector, v.vector);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << (sizeof(SV) / sizeof(T));
                res |= sv_elements[i - 1] < v.sv_elements[i - 1];
            }
            return res;
            #endif
        }

        forceinline
        M operator>(const V &v) const {
            #ifdef __AVX512F__
            V res(0);
            if constexpr (registerSize == RegisterSize::_32bit) {
                return _mm512_cmpgt_epu32_mask(vector, v.vector);
            } else {
                return _mm512_cmpgt_epu64_mask(vector, v.vector);
            }
            #else
            M res(0);
            for (size_t i = n_sv_elements; i > 0; i--) {
                res = res << (sizeof(SV) / sizeof(T));
                res |= sv_elements[i - 1] > v.sv_elements[i - 1];
            }
            return res;
            #endif
        }

        forceinline
        V gather(const void *base_address, const M mask) const {
            V res(0);
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_mask_i32gather_epi32(res.vector, mask, vector, base_address, sizeof(T)));
            } else {
                return V(_mm512_mask_i64gather_epi64(res.vector, mask, vector, base_address, sizeof(T)));
            }
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].gather(base_address, mask >> (i * sizeof(SV) / sizeof(T)));
            }
            return res;
            #endif
        }

        forceinline
        void scatter(void *base_address, const V &index, const M mask) const {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                _mm512_mask_i32scatter_epi32(base_address, mask, index.vector, vector, sizeof(T));
            } else {
                _mm512_mask_i64scatter_epi64(base_address, mask, index.vector, vector, sizeof(T));
            }
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                sv_elements[i].scatter(base_address, index.sv_elements[i], mask >> (i * sizeof(SV) / sizeof(T)));
            }
            #endif
        }

        forceinline
        static V load(const void *address) {
            #ifdef __AVX512F__
            return V(_mm512_load_si512(address));
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = SV::load(reinterpret_cast<const uint8_t *>(address) + sizeof(SV) * i);
            }
            return res;
            #endif
        }

        forceinline
        void store(void *address) {
            #ifdef __AVX512F__
            _mm512_store_si512(address, vector);
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                sv_elements[i].store(reinterpret_cast<const uint8_t *>(address) + sizeof(SV) * i);
            }
            #endif
        }

        template<size_t n_lanes>
        forceinline
        V gatherload(const void *base_address, const M mask) const {
            static_assert(n_lanes == 1 or n_lanes == 2 or n_lanes == 4 or n_lanes == 8 or
                          (n_lanes == 16 and registerSize == RegisterSize::_32bit), "unsupported number of lanes!");

            #ifdef __AVX512F__
            const V zero(0);
            if constexpr (n_lanes == 1 && registerSize == RegisterSize::_64bit) {
                return V(_mm512_mask_load_epi64(zero.vector, mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]));

            } else if constexpr (n_lanes == 1 && registerSize == RegisterSize::_32bit) {
                return V(_mm512_mask_load_epi32(zero.vector, mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]));

            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_64bit) {
                const __m256i lower = _mm256_mask_load_epi64(_mm256_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]);
                const __m256i upper = _mm256_mask_load_epi64(_mm256_set1_epi32(0), mask >> 4,
                        reinterpret_cast<const T *>(base_address) + elements[4]);
                return V(_mm512_inserti64x4(_mm512_inserti64x4(zero.vector, lower, 0), upper, 1));

            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_32bit) {
                const __m256i lower = _mm256_mask_load_epi32(_mm256_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]);
                const __m256i upper = _mm256_mask_load_epi32(_mm256_set1_epi32(0), mask >> 8,
                        reinterpret_cast<const T *>(base_address) + elements[8]);
                return V(_mm512_inserti32x8(_mm512_inserti32x8(zero.vector, lower, 0), upper, 1));

            } else if constexpr (n_lanes == 4 && registerSize == RegisterSize::_64bit) {
                const __m128i lower0 = _mm_mask_load_epi64(_mm_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]);
                const __m128i lower1 = _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 2,
                        reinterpret_cast<const T *>(base_address) + elements[2]);
                const __m128i upper0 = _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 4,
                        reinterpret_cast<const T *>(base_address) + elements[4]);
                const __m128i upper1 = _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 6,
                        reinterpret_cast<const T *>(base_address) + elements[6]);

                return V(_mm512_inserti64x2(
                        _mm512_inserti64x2(_mm512_inserti64x2(_mm512_inserti64x2(zero.vector, lower0, 0), lower1, 1),
                                upper0, 2), upper1, 3));

            } else if constexpr (n_lanes == 4 && registerSize == RegisterSize::_32bit) {
                const __m128i lower0 = _mm_mask_load_epi32(_mm_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + elements[0]);
                const __m128i lower1 = _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 4,
                        reinterpret_cast<const T *>(base_address) + elements[4]);
                const __m128i upper0 = _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 8,
                        reinterpret_cast<const T *>(base_address) + elements[8]);
                const __m128i upper1 = _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 12,
                        reinterpret_cast<const T *>(base_address) + elements[12]);

                return V(_mm512_inserti32x4(
                        _mm512_inserti32x4(_mm512_inserti32x4(_mm512_inserti32x4(zero.vector, lower0, 0), lower1, 1),
                                upper0, 2), upper1, 3));

            } else {
                return this->gather(base_address, mask);
            }
            #else
            return this->gather(base_address, mask);
            #endif
        }

        template<size_t n_lanes>
        forceinline
        void scatterstore_or(void *base_address, const V &index, const M mask) const {
            static_assert(n_lanes == 1 or n_lanes == 2 or n_lanes == 4 or n_lanes == 8 or
                          (n_lanes == 8 and registerSize == RegisterSize::_32bit), "unsupported number of lanes!");

            #ifdef __AVX512F__
            if constexpr (n_lanes == 1 && registerSize == RegisterSize::_64bit) {
                const __m512i value = _mm512_or_si512(vector, _mm512_mask_load_epi64(_mm512_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm512_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value);

            } else if constexpr (n_lanes == 1 && registerSize == RegisterSize::_32bit) {
                const __m512i value = _mm512_or_si512(vector, _mm512_mask_load_epi32(_mm512_set1_epi32(0), mask,
                        reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm512_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value);

            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_64bit) {
                const __m256i value_lower = _mm256_or_si256(_mm512_extracti64x4_epi64(vector, 0),
                        _mm256_mask_load_epi64(_mm256_set1_epi32(0), mask,
                                reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm256_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value_lower);

                const __m256i value_upper = _mm256_or_si256(_mm512_extracti64x4_epi64(vector, 1),
                        _mm256_mask_load_epi64(_mm256_set1_epi32(0), mask >> 4,
                                reinterpret_cast<const T *>(base_address) + index.elements[4]));
                _mm256_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[4], mask >> 4,
                        value_upper);

            } else if constexpr (n_lanes == 2 && registerSize == RegisterSize::_32bit) {
                const __m256i value_lower = _mm256_or_si256(_mm512_extracti32x8_epi32(vector, 0),
                        _mm256_mask_load_epi32(_mm256_set1_epi32(0), mask,
                                reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm256_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value_lower);

                const __m256i value_upper = _mm256_or_si256(_mm512_extracti32x8_epi32(vector, 1),
                        _mm256_mask_load_epi32(_mm256_set1_epi32(0), mask >> 8,
                                reinterpret_cast<const T *>(base_address) + index.elements[8]));
                _mm256_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[8], mask >> 8,
                        value_upper);

            } else if constexpr (n_lanes == 4 && registerSize == RegisterSize::_64bit) {
                const __m128i value_lower0 = _mm_or_si128(_mm512_extracti64x2_epi64(vector, 0),
                        _mm_mask_load_epi64(_mm_set1_epi32(0), mask,
                                reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value_lower0);

                const __m128i value_lower1 = _mm_or_si128(_mm512_extracti64x2_epi64(vector, 1),
                        _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 2,
                                reinterpret_cast<const T *>(base_address) + index.elements[2]));
                _mm_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[2], mask >> 2, value_lower1);

                const __m128i value_upper0 = _mm_or_si128(_mm512_extracti64x2_epi64(vector, 2),
                        _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 4,
                                reinterpret_cast<const T *>(base_address) + index.elements[4]));
                _mm_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[4], mask >> 4, value_upper0);

                const __m128i value_upper1 = _mm_or_si128(_mm512_extracti64x2_epi64(vector, 3),
                        _mm_mask_load_epi64(_mm_set1_epi32(0), mask >> 6,
                                reinterpret_cast<const T *>(base_address) + index.elements[6]));
                _mm_mask_store_epi64(reinterpret_cast<T *>(base_address) + index.elements[6], mask >> 6, value_upper1);

            } else if constexpr (n_lanes == 4 && registerSize == RegisterSize::_32bit) {
                const __m128i value_lower0 = _mm_or_si128(_mm512_extracti32x4_epi32(vector, 0),
                        _mm_mask_load_epi32(_mm_set1_epi32(0), mask,
                                reinterpret_cast<const T *>(base_address) + index.elements[0]));
                _mm_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[0], mask, value_lower0);

                const __m128i value_lower1 = _mm_or_si128(_mm512_extracti32x4_epi32(vector, 1),
                        _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 4,
                                reinterpret_cast<const T *>(base_address) + index.elements[4]));
                _mm_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[4], mask >> 4, value_lower1);

                const __m128i value_upper0 = _mm_or_si128(_mm512_extracti32x4_epi32(vector, 2),
                        _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 8,
                                reinterpret_cast<const T *>(base_address) + index.elements[8]));
                _mm_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[8], mask >> 8, value_upper0);

                const __m128i value_upper1 = _mm_or_si128(_mm512_extracti32x4_epi32(vector, 3),
                        _mm_mask_load_epi32(_mm_set1_epi32(0), mask >> 12,
                                reinterpret_cast<const T *>(base_address) + index.elements[12]));
                _mm_mask_store_epi32(reinterpret_cast<T *>(base_address) + index.elements[12], mask >> 12,
                        value_upper1);

            } else {
                for (size_t i = 0; i < n_sv_elements; i++) {
                    sv_elements[i].template scatterstore_or<n_lanes / n_sv_elements>(base_address, index.sv_elements[i],
                            mask >> (i * sizeof(SV) / sizeof(T)));
                }
            }
            #else
            for (size_t i = 0; i < n_sv_elements; i++) {
                sv_elements[i].template scatterstore_or<(n_lanes + 1) / n_sv_elements>(base_address,
                        index.sv_elements[i], mask >> (i * sizeof(SV) / sizeof(T)));
            }
            #endif
        }

        forceinline
        Vector popcount() {
            #ifdef __AVX512F__
            const __m512i lookup = _mm512_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0, 4, 3, 3, 2, 3, 2, 2,
                    1, 3, 2, 2, 1, 2, 1, 1, 0, 4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0, 4, 3, 3, 2, 3, 2, 2, 1,
                    3, 2, 2, 1, 2, 1, 1, 0);
            const __m512i low_mask = _mm512_set1_epi8(0x0f);
            const __m512i lo = _mm512_and_si512(vector, low_mask);
            const __m512i hi = _mm512_and_si512(_mm512_srli_epi16(vector, 4), low_mask);
            const __m512i cnt1 = _mm512_shuffle_epi8(lookup, lo);
            const __m512i cnt2 = _mm512_shuffle_epi8(lookup, hi);
            const __m512i cnt = _mm512_add_epi8(cnt1, cnt2);
            if constexpr (registerSize == RegisterSize::_64bit) {
                return Vector(_mm512_sad_epu8(cnt, _mm512_setzero_si512()));
            } else {
                const Vector i1 = Vector(cnt);
                const Vector i2 = i1 + (i1 >> 8);
                return (i2 + (i2 >> 16)) & Vector(0xFF);
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = sv_elements[i].popcount();
            }
            return res;
            #endif
        }

        forceinline
        static V loadu(const void *address, const M mask) {
            #ifdef __AVX512F__
            if constexpr (registerSize == RegisterSize::_32bit) {
                return V(_mm512_mask_loadu_epi32(_mm512_set1_epi32(0), mask, address));
            } else {
                return V(_mm512_mask_loadu_epi64(_mm512_set1_epi64(0), mask, address));
            }
            #else
            V res(0);
            for (size_t i = 0; i < n_sv_elements; i++) {
                res.sv_elements[i] = SV::loadu(reinterpret_cast<const uint8_t *>(address) + sizeof(SV) * i,
                        mask >> (i * sizeof(SV) / sizeof(T)));
            }
            return res;
            #endif
        }

        forceinline
        static M mask(bool init) {
            V res(0);
            if constexpr (registerSize == RegisterSize::_32bit) {
                return 0xFFFF * init;
            } else {
                return 0xFF * init;
            }
        }

        forceinline
        static T popcount_mask(M mask) {
            return _mm_popcnt_u32(mask);
        }
    };

} // filters::simd