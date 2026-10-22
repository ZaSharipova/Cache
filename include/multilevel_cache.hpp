#ifndef MULTILEVEL_CACHE_HPP_
#define MULTILEVEL_CACHE_HPP_

#include "ICache.hpp"

#include <cstddef>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
class MultiLevelCache : public ICache<K, V> {
public:
    using key_type = K;
    using value_type = V;

    MultiLevelCache(std::vector<std::unique_ptr<ICache<K, V>>> levels, std::vector<size_t> costs)
                  : levels_(std::move(levels)), costs_(std::move(costs)), level_hits_(levels_.size(), 0) {}

    void Put(K key, V value) override {
        for (auto& level : levels_) {
            level->Put(key, value); // дублирую, потому что использую inclusive способ решения проблемы удаления данных при вытеснении из более "младшего" уровня
        }
    }

    std::string GetName() const override {
        std::string name = levels_[0]->GetName();
        for (size_t i = 1; i < levels_.size(); i++) {
            name += (" -> " + levels_[i]->GetName());
        }

        return name;
    }

    size_t GetCost() const {
        return std::inner_product(level_hits_.begin(), level_hits_.end(), costs_.begin(),
                                  this->misses_ * costs_.back());
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

    std::optional<V> DoGet(const K& key) override {
        for (size_t i = 0; i < levels_.size(); i++) {
            std::optional<V> value = levels_[i]->Get(key);
            if (!value.has_value()) {
                continue;
            }

            level_hits_[i]++;

            for (size_t j = 0; j < i; j++) {
                levels_[j]->Put(key, *value);
            }

            return value;
        }

        return std::nullopt;
    }
};

#endif // MULTILEVEL_CACHE_HPP_
