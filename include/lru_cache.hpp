#ifndef LRU_CACHE_HPP_
#define LRU_CACHE_HPP_

#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>

#include "ICache.hpp"

template<typename K, typename V>
class LRUCache : public ICache<K, V> {
public:
    using key_type = K;
    using value_type = V;

    using ICache<K, V>::evictions_;

    LRUCache(size_t capacity) : capacity_(capacity) {}

    void Put(K key, V value) override {
        if (auto it = pos_.find(key); it != pos_.end()) {
            it->second->second = value;
            order_.splice(order_.begin(), order_, it->second);
            return;
        }

        if (order_.size() >= capacity_) {
            K victim = order_.back().first;
            order_.pop_back();
            pos_.erase(victim);

            evictions_++;
        }

        order_.push_front({key, value});
        pos_[key] = order_.begin();
    }

    std::string GetName() const override {
        return "LRU";
    }

private:

    std::optional<V> DoGet(const K& key) override {
        auto it = pos_.find(key);
        if (it == pos_.end()) {
            return std::nullopt;
        }

        order_.splice(order_.begin(), order_, it->second);

        return it->second->second;
    }

    size_t capacity_ = 1;
    std::list<std::pair<K, V>> order_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> pos_;
};

#endif // LRU_CACHE_HPP_
