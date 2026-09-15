#include "lirs_cache.hpp"

#include <gtest/gtest.h>

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

    for (int key = 100; key < 200; key++) {
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

    for (int key = 100; key < 110; key++) {
        cache.Put(key, key);
    }

    EXPECT_FALSE(cache.Get(100).has_value());
    EXPECT_TRUE(cache.Get(1).has_value());
}
