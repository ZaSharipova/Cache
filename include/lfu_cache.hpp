#ifndef LFU_CACHE_HPP_
#define LFU_CACHE_HPP_

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "ICache.hpp"

template <typename K, typename V>
class LFUCache : public ICache<K, V> {
public:
    using ICache<K, V>::evictions_;

    LFUCache(size_t capacity) : capacity_(capacity) {}

    void Put(K key, V value) override {
        auto it = data_.find(key);
        if (it != data_.end()) {
            Entry& entry = it->second;
            entry.value = value;
            entry.frequency++;
            return;
        }

        if (data_.size() >= capacity_) {
            auto victim = std::min_element(data_.begin(), data_.end(),
                [](const auto& a, const auto& b) {
                    return a.second.frequency < b.second.frequency;
                });

            if (victim != data_.end()) {
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
    struct Entry {
        V value;
        size_t frequency;
    };

    std::optional<V> DoGet(const K& key) override {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return std::nullopt;
        }

        Entry& entry = it->second;
        entry.frequency++;
        return entry.value;
    }

    size_t capacity_ = 1;
    std::unordered_map<K, Entry> data_;
};

#endif // LFU_CACHE_HPP_
