#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "opt.hpp"

#include "subsidiary.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>

namespace {
constexpr int kFirstKey = 1;

constexpr int kMixedHotKeysCount = 5;
constexpr int kMixedScanFirstKey = 100;
constexpr int kMixedScanLastKey = 999;
constexpr int kMixedHotChanceOutOf = 2;

}  // namespace

std::vector<int> MakeScan(int num_keys, int length) { // TODO it's probably better to do functions helper THINK
    std::vector<int> trace;

    trace.reserve(length);
    for (int i = 0; i < length; i++) {
        trace.push_back(i % num_keys + kFirstKey);
    }

    return trace;
}

std::vector<int> MakeMixed(int length, unsigned seed) {
    srand(seed);

    std::vector<int> trace;

    trace.reserve(length);
    int scan_key = kMixedScanFirstKey;
    for (int i = 0; i < length; i++) {
        if (rand() % kMixedHotChanceOutOf == 0) {
            trace.push_back(rand() % kMixedHotKeysCount + kFirstKey);
        } else {
            trace.push_back(scan_key++);
            if (scan_key > kMixedScanLastKey) scan_key = kMixedScanFirstKey;
        }
    }

    return trace;
}

template <typename CacheT>
double HitRatio(CacheT& cache, const std::vector<int>& trace) {
    for (int key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }

    double total = cache.GetHits() + cache.GetMisses();
    return total > 0 ? cache.GetHits() / total : 0.0;
}

double OptRatio(const std::vector<int>& trace, size_t cache_size) {
    return (double)OPT(trace, cache_size) / trace.size();
}

void RunPattern(const std::string& name, const std::vector<int>& trace, size_t capacity) {
    double lru = 0.0, lfu = 0.0, twoq = 0.0, lirs = 0.0, opt = 0.0;
    {
        LRUCache cache(capacity);
        lru = HitRatio(cache, trace);
    }
    {
        LFUCache cache(capacity);
        lfu = HitRatio(cache, trace);
    }
    {
        TwoQCache cache(capacity);
        twoq = HitRatio(cache, trace);
    }
    {
        LIRSCache cache(capacity);
        lirs = HitRatio(cache, trace);
    }

    opt = OptRatio(trace, capacity);

    std::cout << std::left << std::setw(10) << name
              << std::right << std::fixed << std::setprecision(3)
              << std::setw(8) << lru
              << std::setw(8) << lfu
              << std::setw(8) << twoq
              << std::setw(8) << lirs
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
              << std::setw(8) << "OPT" << "\n";

    std::cout << std::string(50, '-') << "\n";

    RunPattern("scan", MakeScan(20, length), capacity);
    RunPattern("hot", MakeHot(100, length, 42), capacity);
    RunPattern("mixed", MakeMixed(length, 42), capacity);

    std::cout << "\n(numbers = hit ratio, proportion of hits; OPT = theoretical maximum)\n";
    return 0;
}
