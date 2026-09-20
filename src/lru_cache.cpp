#include "lru_cache.hpp"

#include <iostream>
#include <list>
#include <optional>

std::optional<int> LRUCache::Get(int key) {
    auto it = pos_.find(key);
    if (it == pos_.end()) {
        misses_++;
        return std::nullopt;
    }

    hits_++;
    order_.splice(order_.begin(), order_, it->second);

    return it->second->second;
}

void LRUCache::Put(int key, int value) {
    auto it = pos_.find(key);
    if (it != pos_.end()) {
        it->second->second = value; // если честно не знаю, насколько уместно здесь на entry переходить
        order_.splice(order_.begin(), order_, it->second);
        return;
    }

    if (order_.size() >= capacity_) {
        int victim = order_.back().first;
        order_.pop_back();
        pos_.erase(victim);

        evictions_++;
    }

    order_.push_front({key, value});
    pos_[key] = order_.begin();
}

size_t LRUCache::GetHits() const {
    return hits_;
}

size_t LRUCache::GetMisses() const {
    return misses_;
}

size_t LRUCache::GetEvictions() const {
    return evictions_;
}
