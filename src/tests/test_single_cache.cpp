#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "opt.hpp"
#include "arc_cache.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <vector>

namespace {
template <typename CacheT>
void Access(CacheT& cache, int key) {
    if (!cache.Get(key).has_value()) {
        cache.Put(key, key);
    }
}

}  // namespace

TEST(LRUCacheTest, BasicPutGet) {
    LRUCache<int, int> cache(2);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(LRUCacheTest, GetMissingReturnsNullopt) {
    LRUCache<int, int> cache(2);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(LRUCacheTest, UpdatesExistingValue) {
    LRUCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(LRUCacheTest, EvictsLeastRecentlyUsed) {
    LRUCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_TRUE(cache.Get(2).has_value());
    EXPECT_TRUE(cache.Get(3).has_value());
}

TEST(LRUCacheTest, GetRefreshesRecency) {
    LRUCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Get(1);
    cache.Put(3, 30);
    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_FALSE(cache.Get(2).has_value());
}

TEST(LRUCacheTest, CapacityOne) {
    LRUCache<int, int> cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2).value_or(-1), 20);
}

TEST(LFUCacheTest, BasicPutGet) {
    LFUCache<int, int> cache(2);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(LFUCacheTest, GetMissingReturnsNullopt) {
    LFUCache<int, int> cache(2);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(LFUCacheTest, UpdatesExistingValue) {
    LFUCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(LFUCacheTest, EvictsLeastFrequentlyUsed) {
    LFUCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Get(1);
    cache.Put(3, 30);
    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_FALSE(cache.Get(2).has_value());
    EXPECT_TRUE(cache.Get(3).has_value());
}

TEST(LFUCacheTest, CapacityOne) {
    LFUCache<int, int> cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2).value_or(-1), 20);
}

TEST(TwoQCacheTest, BasicPutGet) {
    TwoQCache<int, int> cache(8);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(TwoQCacheTest, SmallCapacity) {
    TwoQCache<int, int> cache(3);

    for (int k = 1; k <= 20; k++) {
        if (!cache.Get(k).has_value()) {
            cache.Put(k, k * 10);
        }
    }

    cache.Put(99, 990);
    EXPECT_EQ(cache.Get(99).value_or(-1), 990);
}

TEST(TwoQCacheTest, GetMissingReturnsNullopt) {
    TwoQCache<int, int> cache(8);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(TwoQCacheTest, UpdatesExistingValue) {
    TwoQCache<int, int> cache(8);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(TwoQCacheTest, EvictsFromInputQueue) {
    TwoQCache<int, int> cache(8);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_FALSE(cache.Get(1).has_value());
}

TEST(TwoQCacheTest, ReinsertAfterEviction) {
    TwoQCache<int, int> cache(8);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    EXPECT_FALSE(cache.Get(1).has_value());
    cache.Put(1, 100);
    EXPECT_EQ(cache.Get(1).value_or(-1), 100);
}

TEST(OptTest, CyclicTrace) {
    std::vector<int> trace = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    EXPECT_EQ((OPT<int, int>(trace, 2)), 3);
}

TEST(OptTest, RepeatedKey) {
    std::vector<int> trace = {1, 1, 1, 1};
    EXPECT_EQ((OPT<int, int>(trace, 2)), 3);
}

TEST(LIRSCacheTest, BasicPutGet) {
    LIRSCache<int, int> cache(4);
    cache.Put(1, 10);
    EXPECT_EQ(cache.Get(1).value_or(-1), 10);
}

TEST(LIRSCacheTest, GetMissingReturnsNullopt) {
    LIRSCache<int, int> cache(4);
    EXPECT_FALSE(cache.Get(99).has_value());
}

TEST(LIRSCacheTest, UpdatesExistingValue) {
    LIRSCache<int, int> cache(4);
    cache.Put(1, 10);
    cache.Put(1, 20);
    EXPECT_EQ(cache.Get(1).value_or(-1), 20);
}

TEST(LIRSCacheTest, CapacityOne) {
    LIRSCache<int, int> cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2).value_or(-1), 20);
}

TEST(LIRSCacheTest, MissingKeyIncrementsMisses) {
    LIRSCache<int, int> cache(4);
    EXPECT_EQ(cache.GetMisses(), 0U);
    cache.Get(42);
    EXPECT_EQ(cache.GetMisses(), 1U);
    cache.Get(42);
    EXPECT_EQ(cache.GetMisses(), 2U);
}

TEST(LIRSCacheTest, HitIncrementsHits) {
    LIRSCache<int, int> cache(4);
    cache.Put(1, 10);
    EXPECT_EQ(cache.GetHits(), 0U);
    cache.Get(1);
    EXPECT_EQ(cache.GetHits(), 1U);
    cache.Get(1);
    EXPECT_EQ(cache.GetHits(), 2U);
}

TEST(LIRSCacheTest, HitsAndMissesAreIndependentCounters) {
    LIRSCache<int, int> cache(4);
    cache.Put(1, 10);
    cache.Get(1); // hit
    cache.Get(2); // miss
    cache.Get(1); // hit
    EXPECT_EQ(cache.GetHits(), 2U);
    EXPECT_EQ(cache.GetMisses(), 1U);
}

TEST(LIRSCacheTest, EvictionsHappenWhenOverCapacity) {
    LIRSCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    EXPECT_EQ(cache.GetEvictions(), 0U);

    cache.Put(3, 30);
    cache.Put(4, 40);
    cache.Put(5, 50);

    EXPECT_GT(cache.GetEvictions(), 0U);
}

TEST(LIRSCacheTest, ReinsertAfterEviction) {
    LIRSCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    cache.Put(4, 40);
    cache.Put(5, 50);

    cache.Put(1, 100);
    EXPECT_EQ(cache.Get(1).value_or(-1), 100);
}

TEST(LIRSCacheTest, FrequentlyAccessedKeySurvivesScan) {
    LIRSCache<int, int> cache(4);

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
    LIRSCache<int, int> cache(4);

    cache.Put(1, 10);
    cache.Get(1);
    cache.Get(1);

    for (size_t key = 100; key < 110; key++) {
        cache.Put(key, key);
    }

    EXPECT_FALSE(cache.Get(100).has_value());
    EXPECT_TRUE(cache.Get(1).has_value());
}

TEST(LIRSCacheTest, ReinsertedGhostIsResidentHit) {
    LIRSCache<int, int> cache(2);
    cache.Put(1, 10);
    cache.Put(2, 20);

    for (size_t key = 3; key < 9; key++) {
        cache.Put(key, key * 10);
    }

    size_t misses_before = cache.GetMisses();
    size_t hits_before = cache.GetHits();

    cache.Put(1, 111);

    EXPECT_EQ(cache.Get(1).value_or(-1), 111);
    EXPECT_EQ(cache.GetHits(), hits_before + 1);
    EXPECT_EQ(cache.GetMisses(), misses_before);
}

TEST(LIRSCacheTest, HitsPlusMissesEqualsNumberOfGets) {
    LIRSCache<int, int> cache(3);
    size_t gets = 0;

    for (size_t i = 0; i < 50; i++) {
        cache.Put(i % 5, i);
        cache.Get(i % 7);
        gets++;
        cache.Get(i % 3);
        gets++;
    }

    EXPECT_EQ(cache.GetHits() + cache.GetMisses(), gets);
}

TEST(LIRSCacheTest, ManyEvictionsRemainConsistent) {
    LIRSCache<int, int> cache(3);
    cache.Put(1, 10);
    cache.Get(1);
    cache.Get(1);

    size_t last = 0;

    for (size_t key = 100; key < 600; key++) {
        cache.Put(key, key);
        if (key % 3 == 0) {
            cache.Get(key);
        }
        cache.Put(1, key);
        last = key;
    }

    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(1).value_or(-1), last);
    EXPECT_GT(cache.GetEvictions(), 0U);
}

TEST(LIRSCacheTest, GhostReentrySurvivesLaterScan) {
    LIRSCache<int, int> cache(4);
    cache.Put(1, 10);
    cache.Get(1);
    cache.Get(1);

    for (size_t key = 100; key < 130; key++) {
        cache.Put(key, key);
    }

    cache.Put(1, 700);

    for (size_t key = 200; key < 230; key++) {
        cache.Put(key, key);
    }

    EXPECT_TRUE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(1).value_or(-1), 700);
}

TEST(ARCCache, GetOnEmptyIsMiss) {
    ARCCache<int, int> cache(3);
    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.GetMisses(), 1u);
    EXPECT_EQ(cache.GetHits(), 0u);
}

TEST(ARCCache, PutThenGet) {
    ARCCache<int, int> cache(3);
    cache.Put(1, 10);

    auto result = cache.Get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10);
    EXPECT_EQ(cache.GetHits(), 1u);
}

TEST(ARCCache, PutUpdatesExistingValue) {
    ARCCache<int, int> cache(3);
    cache.Put(1, 10);
    cache.Put(1, 20);

    EXPECT_EQ(cache.Get(1), 20);
    EXPECT_EQ(cache.Size(), 1u);
}

TEST(ARCCache, UpdateInT2KeepsValue) {
    ARCCache<int, int> cache(3);
    cache.Put(1, 10);
    cache.Get(1);
    cache.Put(1, 30);

    EXPECT_EQ(cache.Get(1), 30);
    EXPECT_TRUE(cache.CheckInvariants());
}

TEST(ARCCache, ZeroCapacityStoresNothing) {
    ARCCache<int, int> cache(0);
    cache.Put(1, 10);

    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Size(), 0u);
    EXPECT_TRUE(cache.CheckInvariants());
}

TEST(ARCCache, CapacityOneKeepsLastKey) {
    ARCCache<int, int> cache(1);
    cache.Put(1, 10);
    cache.Put(2, 20);

    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2), 20);
    EXPECT_TRUE(cache.CheckInvariants());
}

TEST(ARCCache, EvictsOldestFromT1) {
    ARCCache<int, int> cache(2);
    cache.Put(1, 1);
    cache.Put(2, 2);
    cache.Put(3, 3);

    EXPECT_FALSE(cache.Get(1).has_value());
    EXPECT_EQ(cache.Get(2), 2);
    EXPECT_EQ(cache.Get(3), 3);
    EXPECT_EQ(cache.GetEvictions(), 1u);
}

TEST(ARCCache, GhostHitIsMissAndPromotesToT2) {
    ARCCache<int, int> cache(2);
    cache.Put(1, 1);
    cache.Put(2, 2);
    cache.Get(1);
    cache.Put(3, 3);
    EXPECT_EQ(cache.GetEvictions(), 1u);

    size_t misses_before = cache.GetMisses();
    EXPECT_FALSE(cache.Get(2).has_value());
    EXPECT_EQ(cache.GetMisses(), misses_before + 1);

    cache.Put(2, 20);
    EXPECT_EQ(cache.GetEvictions(), 2u);
    EXPECT_EQ(cache.Size(), 2u);
    EXPECT_TRUE(cache.CheckInvariants());

    EXPECT_EQ(cache.Get(2), 20);
    EXPECT_EQ(cache.Get(3), 3);
    EXPECT_FALSE(cache.Get(1).has_value());
}

TEST(ARCCache, ScanDoesNotEvictFrequentKeys) {
    ARCCache<int, int> cache(4);
    cache.Put(1, 1);
    cache.Put(2, 2);
    cache.Get(1);
    cache.Get(2);

    for (size_t key = 10; key < 30; key++) {
        Access(cache, key);
    }

    EXPECT_EQ(cache.Get(1), 1);
    EXPECT_EQ(cache.Get(2), 2);
    EXPECT_TRUE(cache.CheckInvariants());
}

TEST(ARCCache, BeatsLruOnScanWithHotKeys) {
    const size_t capacity = 4;
    ARCCache<int, int> arc(capacity);
    LRUCache<int, int> lru(capacity);

    for (size_t pass = 0; pass < 2; pass++) {
        for (int hot : {1, 2}) {
            Access(arc, hot);
            Access(lru, hot);
        }
    }

    for (size_t round = 0; round < 50; round++) {
        for (size_t i = 0; i < 6; i++) {
            Access(arc, 100 + round * 6 + i);
            Access(lru, 100 + round * 6 + i);
        }

        for (size_t hot : {1, 2}) {
            Access(arc, hot);
            Access(lru, hot);
        }
    }

    EXPECT_GT(arc.GetHits(), lru.GetHits());
}

TEST(ARCCache, StatsAddUpToNumberOfGets) {
    ARCCache<int, int> cache(5);

    const size_t gets = 1000;
    for (size_t i = 0; i < gets; i++) {
        Access(cache, (i * 7) % 20 + 1);  // это псевдо-перемешанные ключи 1...20
    }
    EXPECT_EQ(cache.GetHits() + cache.GetMisses(), static_cast<size_t>(gets));
}

class ARCInvariants : public ::testing::TestWithParam<size_t> {};

TEST_P(ARCInvariants, HoldOnCyclicTrace) {
    const size_t capacity = GetParam();
    const int keys = static_cast<int>(capacity) * 3 + 1;
    ARCCache<int, int> cache(capacity);

    for (size_t i = 0; i < 5000; i++) {
        Access(cache, i % keys + 1);
        ASSERT_TRUE(cache.CheckInvariants()) << "capacity=" << capacity << ", step=" << i;
    }
}

TEST_P(ARCInvariants, HoldOnScrambledTrace) {
    const size_t capacity = GetParam();
    const int keys = static_cast<int>(capacity) * 3 + 1;
    ARCCache<int, int> cache(capacity);

    for (size_t i = 0; i < 5000; i++) {
        Access(cache, (i * i) % keys + 1);
        ASSERT_TRUE(cache.CheckInvariants()) << "capacity=" << capacity << ", step=" << i;
    }
}

TEST_P(ARCInvariants, HoldOnHotPlusScanTrace) {
    const size_t capacity = GetParam();
    ARCCache<int, int> cache(capacity);

    int scan_key = 1000;
    for (size_t i = 0; i < 5000; i++) {
        bool hot = (i % 3 != 0);
        Access(cache, hot ? i % 4 + 1 : scan_key++);
        ASSERT_TRUE(cache.CheckInvariants()) << "capacity=" << capacity << ", step=" << i;
    }
}

INSTANTIATE_TEST_SUITE_P(Capacities, ARCInvariants,
                         ::testing::Values(1u, 2u, 3u, 8u, 50u));

TEST(ARCCache, InvariantsHoldOnScanAndHotMix) {
    ARCCache<int, int> cache(8);
    for (size_t round = 0; round < 200; round++) {
        for (int hot = 1; hot <= 3; hot++) {
            Access(cache, hot);
        }

        for (size_t i = 0; i < 20; i++) {
            Access(cache, 100 + (round * 20 + i) % 500);
        }
        ASSERT_TRUE(cache.CheckInvariants()) << "round=" << round;
    }
}
