#ifndef LRU_CACHE_HPP_
#define LRU_CACHE_HPP_

#include <iostream>
#include <list>
#include <optional>

template<typename K, typename V>
class LRUCache {
public:
    using key_type = K;
    using value_type = V;

    LRUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<V> Get(K key) {
        auto it = pos_.find(key);
        if (it == pos_.end()) {
            misses_++;
            return std::nullopt;
        }

        hits_++;
        order_.splice(order_.begin(), order_, it->second);

        return it->second->second;
    }

    void Put(K key, V value) {
        auto it = pos_.find(key);
        if (it != pos_.end()) {
            it->second->second = value; // если честно не знаю, насколько уместно здесь на entry переходить
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

    size_t GetHits() const {
        return hits_;
    }

    size_t GetMisses() const {
        return misses_;
    }

    size_t GetEvictions() const {
        return evictions_;
    }

private:
    size_t capacity_;
    std::list<std::pair<K, V>> order_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> pos_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};

#endif // LRU_CACHE_HPP_
