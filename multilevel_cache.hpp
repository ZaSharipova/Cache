#ifndef MULTILEVEL_CACHE_H_
#define MULTILEVEL_CACHE_H_

#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "2Qcache.hpp"
#include "opt.hpp"

#include <iostream>
#include <optional>

template <typename c1, typename c2>
class MultiLevelCache {
public:
    MultiLevelCache(size_t l1_size, size_t l2_size) : cache_l1_(l1_size), cache_l2_(l2_size) {}

    std::optional<int> Get(int key) {
        std::optional<int> ans_l1 = cache_l1_.Get(key);
        if (ans_l1.has_value()) {
            hits_l1_++;
            return ans_l1;
        }

        std::optional<int> ans_l2 = cache_l2_.Get(key);
        if (ans_l2.has_value()) {
            hits_l2_++;
            cache_l1_.Put(key, ans_l2.value());
            return ans_l2;
        }

        misses_++;
        return std::nullopt;
    }

    void Put(int key, int value) {
        cache_l1_.Put(key, value);
        cache_l2_.Put(key, value);
    }

    size_t GetCost() const {
            return hits_l1_ * 1 + hits_l2_ * 10 + misses_ * 100;
    }

    size_t GetHitsL1() const {
        return hits_l1_;
    }

    size_t GetHitsL2() const {
        return hits_l2_;
    }

    size_t GetMisses() const {
        return misses_;
    }

private:
    c1 cache_l1_;
    c2 cache_l2_;
    size_t hits_l1_ = 0, hits_l2_ = 0, misses_ = 0;
};

#endif // MULTILEVEL_CACHE_H_
