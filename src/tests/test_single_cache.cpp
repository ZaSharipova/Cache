#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
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

TEST(TwoQCacheTest, SmallCapacity) {
    TwoQCache cache(3);

    for (int k = 1; k <= 20; k++) {
        if (!cache.Get(k).has_value()) {
            cache.Put(k, k * 10);
        }
    }

    cache.Put(99, 990);
    EXPECT_EQ(cache.Get(99).value_or(-1), 990);
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

TEST(LIRSCacheTest, BasicPutGet) {
    LIRSCache cache(4);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(LIRSCacheTest, GetMissingReturnsNullopt) {
    LIRSCache cache(4);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(LIRSCacheTest, UpdatesExistingValue) {
    LIRSCache cache(4);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(LIRSCacheTest, CapacityOne) {
    LIRSCache cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2).value_or(-1), 20);
}

TEST(LIRSCacheTest, MissingKeyIncrementsMisses) {
    LIRSCache cache(4);
    EXPECT_EQ(cache.GetMisses(), 0U);
    cache.Get(42);
    EXPECT_EQ(cache.GetMisses(), 1U);
    cache.Get(42);
    EXPECT_EQ(cache.GetMisses(), 2U);
}

TEST(LIRSCacheTest, HitIncrementsHits) {
    LIRSCache cache(4);
    cache.Put(1, 10);
    EXPECT_EQ(cache.GetHits(), 0U);
    cache.Get(1);
    EXPECT_EQ(cache.GetHits(), 1U);
    cache.Get(1);
    EXPECT_EQ(cache.GetHits(), 2U);
}

TEST(LIRSCacheTest, HitsAndMissesAreIndependentCounters) {
    LIRSCache cache(4);
    cache.Put(1, 10);
    cache.Get(1); // hit
    cache.Get(2); // miss
    cache.Get(1); // hit
    EXPECT_EQ(cache.GetHits(), 2U);
    EXPECT_EQ(cache.GetMisses(), 1U);
}

TEST(LIRSCacheTest, EvictionsHappenWhenOverCapacity) {
    LIRSCache cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_EQ(cache.GetEvictions(), 0U);

    cache.Put(3, 30);
    cache.Put(4, 40);
    cache.Put(5, 50);

    EXPECT_GT(cache.GetEvictions(), 0U);
}

TEST(LIRSCacheTest, ReinsertAfterEviction) {
    LIRSCache cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    cache.Put(4, 40);
    cache.Put(5, 50);

    cache.Put(1, 100);
    EXPECT_EQ(cache.Get(1).value_or(-1), 100);
}

TEST(LIRSCacheTest, FrequentlyAccessedKeySurvivesScan) {
    LIRSCache cache(4);

    cache.Put(1, 10);
    cache.Get(1);
    cache.Get(1);
    cache.Get(1);

    for (size_t key = 100; key < 200; key++) {
        cache.Put(key, key);
    }

    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(1).value(), 10);
}

TEST(LIRSCacheTest, OneTimeScanKeysAreEvictedBeforeHotKey) {
    LIRSCache cache(4);

    cache.Put(1, 10);
    cache.Get(1);
    cache.Get(1);

    for (size_t key = 100; key < 110; key++) {
        cache.Put(key, key);
    }

    EXPECT_FALSE(cache.Get(100).has_value());
    EXPECT_TRUE(cache.Get(1).has_value());
}
