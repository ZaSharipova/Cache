#include "lirs_cache.hpp"
#include "opt.hpp"

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

TEST(LIRSCacheTest, OptimalIsUpperBound) {
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

        LIRSCache lirs(cache_size);
        for (int key : trace) {
            if (!lirs.Get(key).has_value()) {
                lirs.Put(key, key);
            }
        }

        EXPECT_GE(OPT(trace, cache_size), lirs.GetHits()) << "failed on trace #" << k;
    }
}
