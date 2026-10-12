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
#include <fstream>

namespace {

struct PatternResult {
    std::string name;
    double lru, lfu, twoq, lirs, arc, opt;
    int seed = 0;
};

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
PatternResult RunPattern(const std::string& name, const std::vector<K>& trace, size_t capacity) {
    PatternResult result;
    result.name = name;
    {
        LRUCache<K, V> cache(capacity);
        result.lru = HitRatio(cache, trace);
    }
    {
        LFUCache<K, V> cache(capacity);
        result.lfu = HitRatio(cache, trace);
    }
    {
        TwoQCache<K, V> cache(capacity);
        result.twoq = HitRatio(cache, trace);
    }
    {
        LIRSCache<K, V> cache(capacity);
        result.lirs = HitRatio(cache, trace);
    }
    {
        ARCCache<K, V> cache(capacity);
        result.arc = HitRatio(cache, trace);
    }
    result.opt = OptRatio<K, V>(trace, capacity);
    return result;
}

void PrintPattern(const PatternResult& result) {
    std::cout << std::left << std::setw(10) << result.name
              << std::right << std::fixed << std::setprecision(3)
              << std::setw(8) << result.lru
              << std::setw(8) << result.lfu
              << std::setw(8) << result.twoq
              << std::setw(8) << result.lirs
              << std::setw(8) << result.arc
              << std::setw(8) << result.opt
              << "\n";
}

void WriteCsv(const std::string& path, const std::vector<PatternResult>& results) {
    std::ofstream out(path);
    out << "pattern,LRU,LFU,2Q,LIRS,ARC,OPT,seed\n";

    for (const PatternResult& result : results) {
        out << result.name << "," << result.lru << "," << result.lfu << "," << result.twoq << ","
            << result.lirs << "," << result.arc << "," << result.opt << ","
            << result.seed << "\n";
    }
}

} // namespace

int main() {
    const size_t capacity = 10;
    const int length = 10000;
    const int n_seeds = 30;

    std::vector<PatternResult> results;
    for (int seed = 1; seed <= n_seeds; seed++) {
        size_t begin = results.size();

        results.push_back(RunPattern<int, int>("scan", MakeScan(20, length), capacity));
        results.push_back(RunPattern<int, int>("hot", MakeHot(100, length, seed), capacity));
        results.push_back(RunPattern<int, int>("mixed", MakeMixed(length, seed), capacity));
        results.push_back(RunPattern<int, int>("phase", MakePhaseShift(length, seed), capacity));

        for (size_t i = begin; i < results.size(); i++) {
            results[i].seed = seed;
        }
    }

    std::cout << "Cache size: " << capacity << ", trace length: " << length
              << ", seeds: " << n_seeds << "\n\n";
    std::cout << std::left << std::setw(10) << "pattern"
              << std::right << std::setw(8) << "LRU"
              << std::setw(8) << "LFU"
              << std::setw(8) << "2Q"
              << std::setw(8) << "LIRS"
              << std::setw(8) << "ARC"
              << std::setw(8) << "OPT" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (const PatternResult& result : results) {
        if (result.seed == 1) {
            PrintPattern(result);
        }
    }

    WriteCsv("csvs/patterns.csv", results);

    std::cout << "\n(numbers = hit ratio for seed 1; OPT = theoretical maximum;"
                 " full data with all seeds in csvs/patterns.csv)\n";
    return 0;
}
