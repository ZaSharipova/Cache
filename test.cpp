#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "opt.hpp"

#include <iostream>
#include <cassert>
#include <vector>

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

void TestOPT() {
    {
        std::vector<int> trace = {1, 2, 3, 1, 2, 3, 1, 2, 3};
        assert(OPT(trace, 2) == 3);
    }
    {
        std::vector<int> trace = {1, 1, 1, 1};
        assert(OPT(trace, 2) == 3);
    }

    {
        const size_t num_traces = 5;
        const size_t trace_length = 1000;
        const int key_range = 20;
        const size_t cache_size = 8;

        srand(42);
        for (size_t k = 0; k < num_traces; k++) {
            std::vector<int> trace;
            for (size_t i = 0; i < trace_length; i++) {
                trace.push_back(rand() % key_range + 1);
            }

            LRUCache lru(cache_size);
            for (size_t i = 0; i < trace.size(); i++) {
                if (!lru.Get(trace[i]).has_value()) {
                    lru.Put(trace[i], trace[i]);
                }
            }

            assert(OPT(trace, cache_size) >= lru.GetHits());
        }
    }

    std::cout << "OPT tests passed\n";
}

int main(void) {
    TestLRU();
    TestLFU();
    TestTwoQ();
    TestOPT();
    std::cout << "\nAll tests passed\n";
    return 0;
}
