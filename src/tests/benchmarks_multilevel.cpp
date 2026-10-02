#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "multilevel_cache.hpp"
#include "opt.hpp"
#include "arc_cache.hpp"

#include "subsidiary.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>

template <typename CacheT, typename K>
void RunMultiLevel(CacheT& cache, const std::vector<K>& trace, const std::string& name) {
    for (K key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }

    size_t total = cache.GetHitsL1() + cache.GetHitsL2() + cache.GetMisses();
    double hit_ratio = static_cast<double>(cache.GetHitsL1() + cache.GetHitsL2()) / total;

    std::cout << std::left << std::setw(14) << name
              << std::right << std::fixed << std::setprecision(3)
              << std::setw(8) << cache.GetHitsL1()
              << std::setw(8) << cache.GetHitsL2()
              << std::setw(8) << cache.GetMisses()
              << std::setw(8) << hit_ratio
              << std::setw(12) << cache.GetCost()
              << "\n";
}

int main() {
    const size_t l1_size = 3;
    const size_t l2_size = 15;
    const int length = 10000;
    const int seed = 42;

    std::vector<int> hot = MakeHot(100, length, seed);

    std::cout << "L1 = " << l1_size << ", L2 = " << l2_size
              << ", pattern hot, trace length: " << length << "\n\n";
    std::cout << std::left << std::setw(14) << "config"
              << std::right << std::setw(8) << "H1"
              << std::setw(8) << "H2"
              << std::setw(8) << "miss"
              << std::setw(8) << "hr"
              << std::setw(12) << "cost" << "\n";
    std::cout << std::string(58, '-') << "\n";

    {
        MultiLevelCache<LRUCache<int, int>, LRUCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "LRU + LRU");
    }
    {
        MultiLevelCache<LRUCache<int, int>, LFUCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "LRU + LFU");
    }
    {
        MultiLevelCache<TwoQCache<int, int>, LFUCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "2Q + LFU");
    }
    {
        MultiLevelCache<LFUCache<int, int>, LFUCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "LFU + LFU");
    }
    {
        MultiLevelCache<LRUCache<int, int>, LIRSCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "LRU + LIRS");
    }
    {
        MultiLevelCache<ARCCache<int, int>, LFUCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "ARC + LFU");
    }
    {
        MultiLevelCache<ARCCache<int, int>, ARCCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "ARC + ARC");
    }
    {
        MultiLevelCache<LRUCache<int, int>, ARCCache<int, int>> cache(l1_size, l2_size);
        RunMultiLevel(cache, hot, "LRU + ARC");
    }

    size_t opt_hits = OPT<int, int>(hot, l1_size + l2_size);
    double opt_ratio = static_cast<double>(opt_hits) / hot.size();
    std::cout << "\nOPT (size L1 + L2 = " << (l1_size + l2_size) << "): "
              << "hits = " << opt_hits << ", hr = "
              << std::fixed << std::setprecision(3) << opt_ratio << "\n";

    return 0;
}
