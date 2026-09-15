#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "opt.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <vector>

TEST(LRUCacheTest, BasicPutGet) {
    LRUCache cache(2);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(LRUCacheTest, GetMissingReturnsNullopt) {
    LRUCache cache(2);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(LRUCacheTest, UpdatesExistingValue) {
    LRUCache cache(2);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(LRUCacheTest, EvictsLeastRecentlyUsed) {
    LRUCache cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_TRUE(cache.Get(2).has_value());
    EXPECT_TRUE(cache.Get(3).has_value());
}

TEST(LRUCacheTest, GetRefreshesRecency) {
    LRUCache cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Get(1);
    cache.Put(3, 30);
    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_FALSE(cache.Get(2).has_value());
}

TEST(LRUCacheTest, CapacityOne) {
    LRUCache cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2).value_or(-1), 20);
}

TEST(LFUCacheTest, BasicPutGet) {
    LFUCache cache(2);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(LFUCacheTest, GetMissingReturnsNullopt) {
    LFUCache cache(2);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(LFUCacheTest, UpdatesExistingValue) {
    LFUCache cache(2);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(LFUCacheTest, EvictsLeastFrequentlyUsed) {
    LFUCache cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Get(1);
    cache.Put(3, 30);
    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_FALSE(cache.Get(2).has_value());
    EXPECT_TRUE(cache.Get(3).has_value());
}

TEST(LFUCacheTest, CapacityOne) {
    LFUCache cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2).value_or(-1), 20);
}

TEST(TwoQCacheTest, BasicPutGet) {
    TwoQCache cache(8);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(TwoQCacheTest, GetMissingReturnsNullopt) {
    TwoQCache cache(8);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(TwoQCacheTest, UpdatesExistingValue) {
    TwoQCache cache(8);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(TwoQCacheTest, EvictsFromInputQueue) {
    TwoQCache cache(8);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_FALSE(cache.Get(1).has_value());
}

TEST(TwoQCacheTest, ReinsertAfterEviction) {
    TwoQCache cache(8);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_FALSE(cache.Get(1).has_value());
    cache.Put(1, 100);
    EXPECT_EQ(cache.Get(1).value_or(-1), 100);
}

TEST(OptTest, CyclicTrace) {
    std::vector<int> trace = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    EXPECT_EQ(OPT(trace, 2), 3);
}

TEST(OptTest, RepeatedKey) {
    std::vector<int> trace = {1, 1, 1, 1};
    EXPECT_EQ(OPT(trace, 2), 3);
}

TEST(OptTest, OptimalIsUpperBoundForLRU) {
    const size_t num_traces = 5;
    const size_t trace_length = 1000;
    const int key_range = 20;
    const size_t cache_size = 8;

    srand(42);
    for (size_t k = 0; k < num_traces; k++) {
        std::vector<int> trace;
        trace.reserve(trace_length);
        for (size_t i = 0; i < trace_length; i++) {
            trace.push_back(rand() % key_range + 1);
        }

        LRUCache lru(cache_size);
        for (int key : trace) {
            if (!lru.Get(key).has_value()) {
                lru.Put(key, key);
            }
        }

        EXPECT_GE(OPT(trace, cache_size), lru.GetHits()) << "failed on trace #" << k;
    }
}
