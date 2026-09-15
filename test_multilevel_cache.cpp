#include "2Qcache.hpp"
#include "lfu_cache.hpp"
#include "lru_cache.hpp"
#include "multilevel_cache.hpp"

#include <gtest/gtest.h>

TEST(MultiLevelCacheTest, NewKeyGoesToL1) {
    MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
    cache.Put(1, 10);

    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
    EXPECT_EQ(cache.GetHitsL1(), 1);
    EXPECT_EQ(cache.GetHitsL2(), 0);
}

TEST(MultiLevelCacheTest, MissingKeyCountsAsMiss) {
    MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
    EXPECT_FALSE(cache.Get(99).has_value());
    EXPECT_EQ(cache.GetMisses(), 1);
}

TEST(MultiLevelCacheTest, KeyPushedOutOfL1IsFoundInL2) {
    MultiLevelCache<LRUCache, LRUCache> cache(1, 4);
    cache.Put(1, 10);
    cache.Put(2, 20);

    auto value = cache.Get(1);
    EXPECT_EQ(value.value_or(-1), 10);
    EXPECT_EQ(cache.GetHitsL2(), 1);
    EXPECT_EQ(cache.GetHitsL1(), 0);
}

TEST(MultiLevelCacheTest, KeyReturnsToL1AfterAccess) {
    MultiLevelCache<LRUCache, LRUCache> cache(1, 4);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Get(1);

    auto value = cache.Get(1);
    EXPECT_EQ(value.value_or(-1), 10);
    EXPECT_GE(cache.GetHitsL1(), 1);
}

TEST(MultiLevelCacheTest, CostIsSummedOverAccesses) {
    MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
    cache.Get(99);
    cache.Put(99, 990);
    cache.Get(99);
    EXPECT_EQ(cache.GetCost(), 101);
}

TEST(MultiLevelCacheTest, RepeatedPutOverwritesValue) {
    MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(MultiLevelCacheTest, WorksWithDifferentCacheTypes) {
    MultiLevelCache<TwoQCache, LFUCache> cache(4, 8);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}
