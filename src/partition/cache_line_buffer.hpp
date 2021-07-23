#pragma once

#include <cstddef>
#include <compiler/compiler_hints.hpp>
#include <immintrin.h>

namespace filters::partition {

    template<typename T>
    struct alignas(64) CacheLineBuffer {
        static constexpr size_t n_tuples_per_cache_line = 64 / sizeof(T);
        static constexpr size_t n_cache_line_buffer = 4;
        static constexpr size_t n_tuples_per_cache_line_buffer = n_tuples_per_cache_line * n_cache_line_buffer;

        T tuples[n_tuples_per_cache_line_buffer];

        forceinline
        void store(T *dst, size_t n_tuples) {
            for (size_t i = 0; i < n_tuples; i++) {
                *(dst + i) = tuples[i];
            }
        }

        forceinline unroll_loops
        void store_non_temporal(void *dst) {
            // Non temporal write of a cache line buffer
            #ifdef __AVX2__
            for (size_t i = 0; i < n_tuples_per_cache_line_buffer / n_tuples_per_cache_line; i++) {
                #ifdef __AVX512F__
                __m512i *d = i + reinterpret_cast<__m512i *>(dst);
                __m512i *s = i + reinterpret_cast<__m512i *>(&tuples[0]);
                _mm512_stream_si512(d, *s);
                #else
                __m256i *d1 = i * 2 + reinterpret_cast<__m256i *>(dst);
                __m256i *s1 = i * 2 + reinterpret_cast<__m256i *>(&tuples[0]);
                __m256i *d2 = d1 + 1;
                __m256i *s2 = s1 + 1;

                _mm256_stream_si256(d1, *s1);
                _mm256_stream_si256(d2, *s2);
                #endif
            };
            #else
            auto *d = reinterpret_cast<CacheLineBuffer *>(dst);
            *d = *this;
            #endif
        }

    };
}
