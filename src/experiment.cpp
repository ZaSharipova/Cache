#include "experiment.hpp"
#include "subsidiary.hpp"
#include "statuses.hpp"

#include "opt.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include <set>
#include <numeric>

constexpr int kHotKeys = 100;

static void Replay(CacheFactory::Cache& cache, const std::vector<int>& trace) {
    for (int key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }
}

Status RunSetup(const CacheFactory::Setup& setup, const std::vector<size_t>& costs,
                const std::vector<int>& trace, int seed, Result& result) {
    std::unique_ptr<CacheFactory::MLCache> cache;
    const Status status = CacheFactory::MakeMultiLevel(setup, costs, cache);
    if (status != Status::kOk) {
        return status;
    }

    Replay(*cache, trace);

    result.name = CacheFactory::Describe(setup);
    result.hits.clear();
    for (size_t level = 0; level < cache->GetLevelCount(); level++) {
        result.hits.push_back(cache->GetHits(level));
    }

    result.misses = cache->GetMisses();
    result.cost = cache->GetCost();
    result.seed = seed;

    const size_t total_hits = std::accumulate(result.hits.begin(), result.hits.end(), size_t{0});
    result.hit_ratio = static_cast<double>(total_hits) / static_cast<double>(trace.size());

    return Status::kOk;
}

Result RunOpt(const std::vector<int>& trace, size_t capacity, int seed) {
    const size_t hits = OPT<int, int>(trace, capacity);

    Result result;
    result.name = "OPT(" + std::to_string(capacity) + ")";
    result.hits = {hits};
    result.misses = trace.size() - hits;
    result.hit_ratio = static_cast<double>(hits) / static_cast<double>(trace.size());
    result.seed = seed;

    return result;
}

static std::set<size_t> DistinctTotalSizes(const std::vector<CacheFactory::Setup>& setups) {
    std::set<size_t> sizes;
    for (const CacheFactory::Setup& setup : setups) {
        sizes.insert(CacheFactory::TotalSize(setup));
    }

    return sizes;
}

Status RunExperiment(const CLI::Config& config, std::vector<Result>& results) {
    const std::set<size_t> opt_capacities = DistinctTotalSizes(config.setups);

    for (int seed = 1; seed <= config.seeds; seed++) {
        const std::vector<int> trace = MakeHot(kHotKeys, config.trace_length, seed);

        for (const CacheFactory::Setup& setup : config.setups) {
            Result result;
            const Status status = RunSetup(setup, config.costs, trace, seed, result);
            if (status != Status::kOk) {
                return status;
            }

            results.push_back(std::move(result));
        }

        for (size_t capacity : opt_capacities) {
            results.push_back(RunOpt(trace, capacity, seed));
        }
    }

    return Status::kOk;
}
