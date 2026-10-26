#include "2Qcache.hpp"
#include "ICache.hpp"
#include "lfu_cache.hpp"
#include "lru_cache.hpp"
#include "multilevel_cache.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace {
using Cache = ICache<int, int>;
using MLCache = MultiLevelCache<int, int>;

template <typename C1, typename C2>
MLCache MakeTwoLevel(size_t s1, size_t s2,
    std::vector<size_t> costs = { 1, 10, 100 }) {
    std::vector<std::unique_ptr<Cache>> levels;
    levels.push_back(std::make_unique<C1>(s1));
    levels.push_back(std::make_unique<C2>(s2));

    return MLCache(std::move(levels), std::move(costs));
}

} // namespace

TEST(MultiLevelCacheTest, NewKeyGoesToL1) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(2, 4);
    cache.Put(1, 10);

    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
    EXPECT_EQ(cache.GetLevelHits(0), 1);
    EXPECT_EQ(cache.GetLevelHits(1), 0);
}

TEST(MultiLevelCacheTest, MissingKeyCountsAsMiss) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(2, 4);
    EXPECT_FALSE(cache.Get(99).has_value());
    EXPECT_EQ(cache.GetMisses(), 1);
}

TEST(MultiLevelCacheTest, KeyPushedOutOfL1IsFoundInL2) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(1, 4);
    cache.Put(1, 10);
    cache.Put(2, 20);

    auto value = cache.Get(1);
    EXPECT_EQ(value.value_or(-1), 10);
    EXPECT_EQ(cache.GetLevelHits(1), 1);
    EXPECT_EQ(cache.GetLevelHits(0), 0);
}

TEST(MultiLevelCacheTest, KeyReturnsToL1AfterAccess) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(1, 4);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Get(1);

    auto value = cache.Get(1);
    EXPECT_EQ(value.value_or(-1), 10);
    EXPECT_GE(cache.GetLevelHits(0), 1);
}

TEST(MultiLevelCacheTest, CostIsSummedOverAccesses) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(2, 4);
    cache.Get(99);
    cache.Put(99, 990);
    cache.Get(99);
    EXPECT_EQ(cache.GetCost(), 101);
}

TEST(MultiLevelCacheTest, RepeatedPutOverwritesValue) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(2, 4);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(MultiLevelCacheTest, WorksWithDifferentCacheTypes) {
    auto cache = MakeTwoLevel<LRUCache<int, int>, LRUCache<int, int>>(4, 8);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(MultiLevelCacheTest, ThreeLevels) {
    std::vector<std::unique_ptr<Cache>> levels;
    levels.push_back(std::make_unique<LRUCache<int, int>>(1));
    levels.push_back(std::make_unique<LRUCache<int, int>>(2));
    levels.push_back(std::make_unique<LRUCache<int, int>>(4));
    MLCache cache(std::move(levels), { 1, 10, 50, 200 });

    for (size_t k = 1; k <= 3; k++) {
        cache.Put(k, k * 10);
    }

    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
    EXPECT_EQ(cache.GetLevelHits(2), 1);
    EXPECT_EQ(cache.GetCost(), 50);
}
