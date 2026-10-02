#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "opt.hpp"
#include "arc_cache.hpp"

#include "subsidiary.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>

template <typename CacheT, typename K>
double HitRatio(CacheT& cache, const std::vector<K>& trace) {
    for (K key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }

    double total = cache.GetHits() + cache.GetMisses();
    return total > 0 ? cache.GetHits() / total : 0.0;
}

template <typename K, typename V>
double OptRatio(const std::vector<K>& trace, size_t cache_size) {
    return (double)OPT<K, V>(trace, cache_size) / trace.size();
}

template <typename K, typename V>
void RunPattern(const std::string& name, const std::vector<K>& trace, size_t capacity) {
    double lru = 0.0, lfu = 0.0, twoq = 0.0, lirs = 0.0, opt = 0.0, arc = 0.0;
    {
        LRUCache<K, V> cache(capacity);
        lru = HitRatio(cache, trace);
    }
    {
        LFUCache<K, V> cache(capacity);
        lfu = HitRatio(cache, trace);
    }
    {
        TwoQCache<K, V> cache(capacity);
        twoq = HitRatio(cache, trace);
    }
    {
        LIRSCache<K, V> cache(capacity);
        lirs = HitRatio(cache, trace);
    }
    {
        ARCCache<K, V> cache(capacity);
        arc = HitRatio(cache, trace);
    }

    opt = OptRatio<K, V>(trace, capacity);

    std::cout << std::left << std::setw(10) << name
              << std::right << std::fixed << std::setprecision(3)
              << std::setw(8) << lru
              << std::setw(8) << lfu
              << std::setw(8) << twoq
              << std::setw(8) << lirs
              << std::setw(8) << arc
              << std::setw(8) << opt
              << "\n";
}

int main() {
    const size_t capacity = 10;
    const int length = 10000;

    std::cout << "Cache size: " << capacity << ", trace length: " << length << "\n\n";
    std::cout << std::left << std::setw(10) << "pattern"
              << std::right << std::setw(8) << "LRU"
              << std::setw(8) << "LFU"
              << std::setw(8) << "2Q"
              << std::setw(8) << "LIRS"
              << std::setw(8) << "ARC"
              << std::setw(8) << "OPT" << "\n";

    std::cout << std::string(60, '-') << "\n";

    RunPattern<int, int>("scan", MakeScan(20, length), capacity);
    RunPattern<int, int>("hot", MakeHot(100, length, 42), capacity);
    RunPattern<int, int>("mixed", MakeMixed(length, 42), capacity);
    RunPattern<int, int>("phase", MakePhaseShift(length, 42), capacity);

    std::cout << "\n(numbers = hit ratio, proportion of hits; OPT = theoretical maximum)\n";
    return 0;
}
