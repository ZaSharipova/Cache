#include "2Qcache.hpp"
#include "lfu_cache.hpp"
#include "lru_cache.hpp"
#include "multilevel_cache.hpp"

#include <iostream>
#include <vector>

void TestMultiLevel() {
    {
        MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
        cache.Put(1, 10);

        assert(cache.Get(1).value_or(-1) == 10);
        assert(cache.GetHitsL1() == 1);
        assert(cache.GetHitsL2() == 0);
    }
    {
        MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
        assert(!cache.Get(99).has_value());
        assert(cache.GetMisses() == 1);
    }
    {
        MultiLevelCache<LRUCache, LRUCache> cache(1, 4);
        cache.Put(1, 10);
        cache.Put(2, 20);
        auto value = cache.Get(1);
        assert(value.value_or(-1) == 10);
        assert(cache.GetHitsL2() == 1);
        assert(cache.GetHitsL1() == 0);
    }
    {
        MultiLevelCache<LRUCache, LRUCache> cache(1, 4);
        cache.Put(1, 10);
        cache.Put(2, 20);
        cache.Get(1);
        auto value = cache.Get(1);
        assert(value.value_or(-1) == 10);
        assert(cache.GetHitsL1() >= 1);
    }
    {
        MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
        cache.Get(99);
        cache.Put(99, 990);
        cache.Get(99);
        assert(cache.GetCost() == 101);
    }
    {
        MultiLevelCache<LRUCache, LRUCache> cache(2, 4);
        cache.Put(1, 10);
        cache.Put(1, 20);
        assert(cache.Get(1).value_or(-1) == 20);
    }
    {
        MultiLevelCache<TwoQCache, LFUCache> cache(4, 8);
        cache.Put(1, 10);
        assert(cache.Get(1).value_or(-1) == 10);
    }

    std::cout << "MultiLevel tests passed\n";
}

int main() {
    TestMultiLevel();
    return 0;
}
