#include "lfu_cache.hpp"

std::optional<int> LFUCache::Get(int key) {
    auto it = data_.find(key);
    if (it == data_.end()) {
        misses_++;
        return std::nullopt;
    }

    hits_++;
    Entry& entry = it->second;
    entry.frequency++;
    return entry.value;
}

void LFUCache::Put(int key, int value) {
    auto it = data_.find(key);
    if (it != data_.end()) {
        Entry& entry = it->second;
        entry.value = value;
        entry.frequency++;
        return;
    }

    if (data_.size() >= capacity_) {
        size_t min_frequency = SIZE_MAX;
        int victim = 0;
        bool found_flag = false;

        for (auto it : data_) {
            Entry& entry = it.second;
            if (entry.frequency < min_frequency) {
                min_frequency = entry.frequency;
                victim = it.first;
                found_flag = true;
            }
        }

        if (found_flag) {
            data_.erase(victim);
            evictions_++;
        }
    }

    data_[key] = {value, 1};
}

size_t LFUCache::GetHits() const {
    return hits_;
}

size_t LFUCache::GetMisses() const {
    return misses_;
}

size_t LFUCache::GetEvictions() const {
    return evictions_;
}
