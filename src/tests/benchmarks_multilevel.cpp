#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "multilevel_cache.hpp"
#include "opt.hpp"
#include "arc_cache.hpp"
#include "subsidiary.hpp"

#include <charconv>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <utility>
#include <cstddef>
#include <fstream>

namespace {

struct Result {
    std::string name;
    size_t h1, h2, misses;
    double hr;
    size_t cost;
    int seed = 0;
};

using Candidates = std::tuple<
    LRUCache<int, int>,
    LFUCache<int, int>,
    TwoQCache<int, int>,
    LIRSCache<int, int>,
    ARCCache<int, int>
>;

constexpr size_t kCandCount = std::tuple_size_v<Candidates>;

template <typename C> struct CacheName;
template <typename K, typename V> struct CacheName<LRUCache<K, V>>  { static constexpr const char* value = "LRU"; };
template <typename K, typename V> struct CacheName<LFUCache<K, V>>  { static constexpr const char* value = "LFU"; };
template <typename K, typename V> struct CacheName<TwoQCache<K, V>> { static constexpr const char* value = "2Q"; };
template <typename K, typename V> struct CacheName<LIRSCache<K, V>> { static constexpr const char* value = "LIRS"; };
template <typename K, typename V> struct CacheName<ARCCache<K, V>>  { static constexpr const char* value = "ARC"; };

void PrintResult(const Result& result) {
    std::cout << std::left << std::setw(14) << result.name
              << std::right << std::fixed << std::setprecision(3)
              << std::setw(8) << result.h1
              << std::setw(8) << result.h2
              << std::setw(8) << result.misses
              << std::setw(8) << result.hr
              << std::setw(12) << result.cost
              << "\n";
}

void WriteCsv(const std::string& path, const std::vector<Result>& results) {
    std::ofstream out(path);
    out << "config,h1,h2,miss,hr,cost,seed\n";
    for (const Result& result : results) {
        out << result.name << "," << result.h1 << "," << result.h2 << "," << result.misses << ","
            << result.hr << "," << result.cost << "," << result.seed << "\n";
    }
}

template <typename CacheT, typename K>
Result RunMultiLevel(CacheT& cache, const std::vector<K>& trace, const std::string& name) {
    for (K key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }

    size_t h1 = cache.GetHits(0);
    size_t h2 = cache.GetHits(1);
    size_t total = h1 + h2 + cache.GetMisses();
    double hit_ratio = static_cast<double>(h1 + h2) / total;

    return {name, h1, h2, cache.GetMisses(), hit_ratio, cache.GetCost()};
}

template <size_t I, size_t J>
void RunPair(const std::vector<int>& trace, size_t l1, size_t l2,
             std::vector<Result>& results) {
    using C1 = std::tuple_element_t<I, Candidates>;
    using C2 = std::tuple_element_t<J, Candidates>;

    MultiLevelCache<C1, C2> cache(l1, l2);
    std::string name = std::string(CacheName<C1>::value) + " + " + CacheName<C2>::value;

    Result result = RunMultiLevel(cache, trace, name);
    results.push_back(result);
}

template <size_t I, size_t... Js>
void RunRow(const std::vector<int>& trace, size_t l1, size_t l2,
            std::vector<Result>& results, std::index_sequence<Js...>) {
    (RunPair<I, Js>(trace, l1, l2, results), ...);
}

template <size_t... Is>
void RunAll(const std::vector<int>& trace, size_t l1, size_t l2,
            std::vector<Result>& results, std::index_sequence<Is...>) {
    (RunRow<Is>(trace, l1, l2, results, std::make_index_sequence<kCandCount>{}), ...);
}

} // namespace

int main() {
    const size_t l1_size = 3;
    const size_t l2_size = 15;
    const int length = 10000;
    const int n_seeds = 30;

    std::vector<Result> results;
    for (int seed = 1; seed <= n_seeds; seed++) {
        std::vector<int> hot = MakeHot(100, length, seed);

        size_t begin = results.size();
        RunAll(hot, l1_size, l2_size, results, std::make_index_sequence<kCandCount>{});
        for (size_t i = begin; i < results.size(); i++) {
            results[i].seed = seed;
        }

        size_t opt_hits = OPT<int, int>(hot, l1_size + l2_size);
        results.push_back({"OPT", opt_hits, 0, hot.size() - opt_hits,
                           static_cast<double>(opt_hits) / hot.size(), 0, seed});
    }

    std::cout << "L1 = " << l1_size << ", L2 = " << l2_size
              << ", pattern hot, trace length: " << length << "\n\n";
    for (const Result& result : results) {
        if (result.seed == 1) {
            PrintResult(result);
        }
    }

    WriteCsv("csvs/results.csv", results);

    return 0;
}
