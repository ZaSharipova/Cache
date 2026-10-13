#ifndef LFU_CACHE_HPP_
#define LFU_CACHE_HPP_

#include <iostream>
#include <unordered_map>
#include <cstdint>
#include <optional>

#include "ICache.hpp"

template <typename K, typename V>
class LFUCache : public ICache<K, V> {
public:
    using key_type = K;
    using value_type = V;

    using ICache<K, V>::hits_;
    using ICache<K, V>::misses_;
    using ICache<K, V>::evictions_;

    LFUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<V> Get(K key) override {
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

    void Put(K key, V value) override {
        auto it = data_.find(key);
        if (it != data_.end()) {
            Entry& entry = it->second;
            entry.value = value;
            entry.frequency++;
            return;
        }

        if (data_.size() >= capacity_) {
            size_t min_frequency = SIZE_MAX;
            K victim = 0;
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

    std::string GetName() const override {
        return "LFU";
    }

private:
    size_t capacity_;

    struct Entry {
        V value;
        size_t frequency;
    };

    std::unordered_map<K, Entry> data_;
};

#endif // LFU_CACHE_HPP_
