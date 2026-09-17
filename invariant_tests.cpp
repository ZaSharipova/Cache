#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "lirs_cache.hpp" // it seems lirs to be included somewhere multiple times FIXME
#include "lru_cache.hpp"
#include "opt.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <vector>

namespace {

std::vector<int> MakeRandomTrace(size_t length, int key_range, unsigned seed) {
    srand(seed);
    std::vector<int> trace;

    trace.reserve(length);
    for (size_t i = 0; i < length; i++) {
        trace.push_back(rand() % key_range + 1);
    }

    return trace;
}

template <typename CacheT>
size_t RunAndGetHits(CacheT& cache, const std::vector<int>& trace) {
    for (int key : trace) {
        if (!cache.Get(key).has_value()) {
            cache.Put(key, key);
        }
    }

    return cache.GetHits();
}

}  // namespace

using CacheTypes = ::testing::Types<LFUCache, TwoQCache, LIRSCache, LRUCache>;

template <typename CacheT>
class InvariantTest : public ::testing::Test {};

TYPED_TEST_SUITE(InvariantTest, CacheTypes);

TYPED_TEST(InvariantTest, OptIsUpperBound) {
    const size_t num_traces = 5;
    const size_t trace_length = 1000;
    const int key_range = 20;
    const size_t cache_size = 8;

    for (size_t k = 0; k < num_traces; k++) {
        std::vector<int> trace = MakeRandomTrace(trace_length, key_range, 42 + k);
        TypeParam cache(cache_size);
        size_t hits = RunAndGetHits(cache, trace);

        EXPECT_GE(OPT(trace, cache_size), hits) << "failed on trace #" << k;
    }
}
