#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "lru_cache.hpp"
#include "opt.hpp"
#include "arc_cache.hpp"

#include "subsidiary.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <vector>

namespace {
constexpr size_t kNumTraces = 5;
constexpr size_t kTraceLength = 1000;
constexpr int kKeyRange = 20;
constexpr size_t kCacheSize = 8;
constexpr unsigned kStartingSeed = 42;

template <typename CacheT>
size_t RunAndGetHits(CacheT& cache, const std::vector<int>& trace) {
    for (int key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }

    return cache.GetHits();
}

} // namespace

using CacheTypes = ::testing::Types<LFUCache, TwoQCache, LIRSCache, LRUCache, ARCCache>;

template <typename CacheT>
class InvariantTest : public ::testing::Test {};

TYPED_TEST_SUITE(InvariantTest, CacheTypes);

TYPED_TEST(InvariantTest, OptIsUpperBound) {
    for (size_t k = 0; k < kNumTraces; k++) {
        std::vector<int> trace = MakeRandomTrace(kTraceLength, kKeyRange, kStartingSeed + k);
        TypeParam cache(kCacheSize);
        size_t hits = RunAndGetHits(cache, trace);

        EXPECT_GE(OPT(trace, kCacheSize), hits) << "failed on trace #" << k;
    }
}
