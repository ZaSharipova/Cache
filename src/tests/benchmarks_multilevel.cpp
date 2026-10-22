#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp"
#include "multilevel_cache.hpp"
#include "opt.hpp"
#include "arc_cache.hpp"
#include "subsidiary.hpp"

#include "report.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using Candidates = std::tuple<
    LRUCache<int, int>,
    LFUCache<int, int>,
    TwoQCache<int, int>,
    LIRSCache<int, int>,
    ARCCache<int, int>
>;

constexpr size_t kCandCount = std::tuple_size_v<Candidates>;

void PrintResult(const Result& result) {
    std::cout << std::left << std::setw(14) << result.name
              << std::right << std::fixed << std::setprecision(3)
              << std::setw(8) << result.hits[0]
              << std::setw(8) << result.hits[1]
              << std::setw(8) << result.misses
              << std::setw(8) << result.hit_ratio
              << std::setw(12) << result.cost
              << "\n";
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

    return {name, {h1, h2}, cache.GetMisses(), hit_ratio, cache.GetCost(), 0};
}
template <size_t I, size_t J>
void RunPair(const std::vector<int>& trace, size_t l1, size_t l2, std::vector<Result>& results) {
    using C1 = std::tuple_element_t<I, Candidates>;
    using C2 = std::tuple_element_t<J, Candidates>;

    std::vector<std::unique_ptr<ICache<int, int>>> levels;
    levels.push_back(std::make_unique<C1>(l1));
    levels.push_back(std::make_unique<C2>(l2));
    std::vector<size_t> costs = {1, 10, 100};

    MultiLevelCache<int, int> cache(std::move(levels), costs);

    Result result = RunMultiLevel(cache, trace, cache.GetName());
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

        size_t opt_hits = OPT<int>(hot, l1_size + l2_size);
        results.push_back({"OPT", {opt_hits}, hot.size() - opt_hits,
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
