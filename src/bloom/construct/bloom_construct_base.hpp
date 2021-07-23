#pragma once

#include <bloom/bloom_parameter.hpp>
#include <partition/partition_set.hpp>
#include <hash/hasher.hpp>
#include <bloom/mask/bloom_mask.hpp>

namespace filters::bloom {

    template<Variant, Vectorization, BlockSize, Groups, size_t, parameter::RegisterSize registerSize, parameter::SIMD _simd,
            typename Container, typename Hasher>
    struct BloomConstructor {

        using Vector = simd::Vector<registerSize, _simd>;
        using T = typename Vector::T;

        forceinline
        void insert(Container &, const Vector &, const size_t) {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        void insert(Container &, const Vector &, const typename Vector::M, const size_t) {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        void construct_partition(Container &, const T *, const size_t, const size_t) {
            throw std::runtime_error{"not implemented!"};
        }

        template<typename PartitionSet>
        forceinline
        void construct(Container &, const PartitionSet &) {
            throw std::runtime_error{"not implemented!"};
        }

        template<typename PartitionSet, typename TaskQueue>
        forceinline
        void construct(Container &, const PartitionSet &, TaskQueue &) {
            throw std::runtime_error{"not implemented!"};
        }
    };

} // filters::bloom