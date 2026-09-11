#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"

void TestLRU() {
    {
        LRUCache cache(2);
        cache.Put(1, 10);
        assert(cache.Get(1).value_or(-1) == 10);
    }
    {
        LRUCache cache(2);
        assert(!cache.Get(99).has_value());
    }
    {
        LRUCache cache(2);
        cache.Put(1, 10);
        cache.Put(1, 20);
        assert(cache.Get(1).value_or(-1) == 20);
    }
    {
        LRUCache cache(2);
        cache.Put(1, 10);
        cache.Put(2, 20);
        cache.Put(3, 30);
        assert(!cache.Get(1).has_value());
        assert(cache.Get(2).has_value());
        assert(cache.Get(3).has_value());
    }
    {
        LRUCache cache(2);
        cache.Put(1, 10);
        cache.Put(2, 20);
        cache.Get(1);
        cache.Put(3, 30);
        assert(cache.Get(1).has_value());
        assert(!cache.Get(2).has_value());
    }
    {
        LRUCache cache(1);
        cache.Put(1, 10);
        cache.Put(2, 20);
        assert(!cache.Get(1).has_value());
        assert(cache.Get(2).value_or(-1) == 20);
    }
    std::cout << "LRU tests passed\n";
}

void TestLFU() {
    {
        LFUCache cache(2);
        cache.Put(1, 10);
        assert(cache.Get(1).value_or(-1) == 10);
    }
    {
        LFUCache cache(2);
        assert(!cache.Get(99).has_value());
    }
    {
        LFUCache cache(2);
        cache.Put(1, 10);
        cache.Put(1, 20);
        assert(cache.Get(1).value_or(-1) == 20);
    }
    {
        LFUCache cache(2);
        cache.Put(1, 10);
        cache.Put(2, 20);
        cache.Get(1);
        cache.Put(3, 30);
        assert(cache.Get(1).has_value());
        assert(!cache.Get(2).has_value());
        assert(cache.Get(3).has_value());
    }
    {
        LFUCache cache(1);
        cache.Put(1, 10);
        cache.Put(2, 20);
        assert(!cache.Get(1).has_value());
        assert(cache.Get(2).value_or(-1) == 20);
    }
    std::cout << "LFU tests passed\n";
}

void TestTwoQ() {
    {
        TwoQCache cache(8);
        cache.Put(1, 10);
        assert(cache.Get(1).value_or(-1) == 10);
    }
    {
        TwoQCache cache(8);
        assert(!cache.Get(99).has_value());
    }
    {
        TwoQCache cache(8);
        cache.Put(1, 10);
        cache.Put(1, 20);
        assert(cache.Get(1).value_or(-1) == 20);
    }
    {
        TwoQCache cache(8);
        cache.Put(1, 10);
        cache.Put(2, 20);
        cache.Put(3, 30);
        assert(!cache.Get(1).has_value());
    }
    {
        TwoQCache cache(8);
        cache.Put(1, 10);
        cache.Put(2, 20);
        cache.Put(3, 30);
        assert(!cache.Get(1).has_value());
        cache.Put(1, 100);
        assert(cache.Get(1).value_or(-1) == 100);
    }
    std::cout << "2Q tests passed\n";
}

int main(void) {
    TestLRU();
    TestLFU();
    TestTwoQ();
    std::cout << "\nAll tests passed\n";
    return 0;
}
