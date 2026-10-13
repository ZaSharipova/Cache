#ifndef MULTILEVEL_CACHE_HPP_
#define MULTILEVEL_CACHE_HPP_

#include "ICache.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
class MultiLevelCache : public ICache<K, V> {
public:
    using key_type = K;
    using value_type = V;

    using ICache<K, V>::hits_;
    using ICache<K, V>::misses_;

    MultiLevelCache(std::vector<std::unique_ptr<ICache<K, V>>> levels, std::vector<size_t> costs)
                  : levels_(std::move(levels)), costs_(std::move(costs)), level_hits_(levels_.size(), 0) {}

    std::optional<V> Get(K key) override {

        for (size_t i = 0; i < levels_.size(); i++) {
            std::optional<V> value = levels_[i]->Get(key);
            if (!value.has_value()) {
                continue;
            }

            hits_++;
            level_hits_[i]++;

            for (size_t j = 0; j < i; j++) {
                levels_[j]->Put(key, *value);
            }

            return value;
        }

        misses_++;
        return std::nullopt;
    }

    void Put(K key, V value) override {
        for (auto& level : levels_) {
            level->Put(key, value); // дублирую, потому что использую inclusive способ решения проблемы удаления данных при вытеснении из более "младшего" уровня
        }
    }

    std::string GetName() const override {
        std::string name;
        for (size_t i = 0; i < levels_.size(); i++) {
            if (i != 0) {
                name += " -> ";
            }

            name += levels_[i]->GetName();
        }

        return name;
    }

    size_t GetCost() const {
        size_t cost = misses_ * costs_.back();
        for (size_t i = 0; i < levels_.size(); i++) {
            cost += level_hits_[i] * costs_[i];
        }

        return cost;
    }

    size_t GetLevelCount() const {
        return levels_.size();
    }

    size_t GetHits(size_t level) const {
        return level_hits_.at(level);
    }

    const ICache<K, V>& GetLevel(size_t level) const {
        return *levels_.at(level);
    }

    size_t GetLevelHits(size_t level) const {
        return level_hits_.at(level);
    }

private:
    std::vector<std::unique_ptr<ICache<K, V>>> levels_;
    std::vector<size_t> costs_;
    std::vector<size_t> level_hits_;
};

#endif // MULTILEVEL_CACHE_HPP_
