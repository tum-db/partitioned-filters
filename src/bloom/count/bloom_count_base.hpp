#pragma once

#include <bloom/bloom_parameter.hpp>
#include <atomic>

namespace filters::bloom {

    template<Variant, Vectorization, BlockSize, Groups, size_t, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomCounter {

        using Vector = simd::Vector<registerSize, _simd>;
        using M = typename Vector::M;

        forceinline
        M contains(const Container &, const Vector &, const size_t) const {
            throw std::runtime_error{"not implemented"};
        }

        forceinline
        M contains(const Container &, const Vector &, const M, const size_t) const {
            throw std::runtime_error{"not implemented"};
        }

        template<typename PartitionSet>
        forceinline
        size_t count(const Container &, const PartitionSet &) const {
            throw std::runtime_error{"not implemented"};
        }

        template<typename PartitionSet, typename TaskQueue>
        forceinline
        void count(const Container &, const PartitionSet &, std::atomic<size_t> &, TaskQueue &) const {
            throw std::runtime_error{"not implemented"};
        }
    };

} // filters::bloom