#pragma once

#include <compiler/compiler_hints.hpp>
#include <simd/vector_div.hpp>
#include <cstddef>

namespace filters::cuckoo {

    static constexpr size_t MAX_ITERATION = 1000;

    template<size_t k, typename Addresser, typename Vector>
    forceinline
    static Vector cuckoo_fingerprint(const Addresser &addresser, const Vector &vector) {
        const Vector fingerprint = addresser.template fingerprint<k>(vector);
        if constexpr (Vector::avx) {
            const Vector zero = Vector(0);
            const Vector one = Vector(1);
            return fingerprint.add_mask(one, fingerprint == zero);
        } else {
            return Vector(fingerprint.vector + (fingerprint.vector == 0));
        }
    }

    template<size_t k, typename Addresser, typename Vector>
    forceinline
    static Vector morton_fingerprint(const Addresser &addresser, const Vector &vector) {
        return addresser.template fingerprint<k>(vector);
    }

    template<size_t k, size_t slots_per_bucket, typename T>
    constexpr T compute_mask() {
        if constexpr (slots_per_bucket == 0) {
            return 0;
        } else {
            return (compute_mask<k, slots_per_bucket - 1, T>() << k) | 0b1;
        }
    }

    template<size_t k, size_t slots_per_bucket, typename T>
    forceinline
    static T has_zero(const T &bucket) {
        constexpr T mask = compute_mask<k, slots_per_bucket, T>();

        return (((bucket - mask) & ~bucket) >> (k - 1)) & mask;
    }

    template<size_t k, size_t slots_per_bucket, typename T, typename F>
    forceinline
    static T has_value(const T &bucket, const F &value) {
        constexpr T mask = compute_mask<k, slots_per_bucket, T>();

        return has_zero<k, slots_per_bucket>(bucket ^ (mask * value));
    }

    template<size_t n_buckets_per_block, typename Vector>
    forceinline
    void split_address(const Vector &address, Vector &block_address, Vector &bucket_idx) {
        return simd::div_mod<n_buckets_per_block>(address, block_address, bucket_idx);
    }
}

