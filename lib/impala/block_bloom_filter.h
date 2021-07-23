// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

namespace kudu {

    // Space and cache efficient block based BloomFilter that takes 32-bit hash as key.
    // For a simple BloomFilter that takes arbitrary datatype as key see BloomFilter in bloom_filter.h
    //
    // A BloomFilter stores sets of items and offers a query operation indicating whether or
    // not that item is in the set.  BloomFilters use much less space than other compact data
    // structures, but they are less accurate: for a small percentage of elements, the query
    // operation incorrectly returns true even when the item is not in the set.
    //
    // When talking about Bloom filter size, rather than talking about 'size', which might be
    // ambiguous, we distinguish two different quantities:
    //
    // 1. Space: the amount of memory used
    //
    // 2. NDV: the number of unique items that have been inserted
    //
    // BlockBloomFilter is implemented using block Bloom filters from Putze et al.'s "Cache-,
    // Hash- and Space-Efficient Bloom Filters". The basic idea is to hash the item to a tiny
    // Bloom filter the size of a single cache line or smaller. This implementation sets 8
    // bits in each tiny Bloom filter. This provides a false positive rate near optimal for
    // between 5 and 15 bits per distinct value, which corresponds to false positive
    // probabilities between 0.1% (for 15 bits) and 10% (for 5 bits).
    //
    // Our tiny BloomFilters are 32 bytes to take advantage of 32-byte SIMD in newer Intel
    // machines.
    class BlockBloomFilter {
      public:
        static constexpr uint64_t kBucketWords = 8;
        typedef uint32_t BucketWord;

        typedef BucketWord Bucket[kBucketWords];

        Bucket *directory_;

        // A faster SIMD version of BucketInsert().
        void BucketInsertAVX2(uint32_t bucket_idx, uint32_t hash) noexcept
        __attribute__((__target__("avx2")));

        // A faster SIMD version of BucketFind().
        bool BucketFindAVX2(uint32_t bucket_idx, uint32_t hash) const noexcept
        __attribute__((__target__("avx2")));

        // Some constants used in hashing. #defined for efficiency reasons.
        #define BLOOM_HASH_CONSTANTS                                             \
  0x47b6137bU, 0x44974d91U, 0x8824ad5bU, 0xa2b7289dU, 0x705495c7U, 0x2df1424bU, \
      0x9efc4947U, 0x5c6bfb31U

        // kRehash is used as 8 odd 32-bit unsigned ints.  See Dietzfelbinger et al.'s "A
        // reliable randomized algorithm for the closest-pair problem".
        static constexpr uint32_t kRehash[8]
                __attribute__((aligned(32))) = {BLOOM_HASH_CONSTANTS};
    };

}  // namespace kudu