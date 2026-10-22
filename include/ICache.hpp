#ifndef ICACHE_HPP_
#define ICACHE_HPP_

#include <cstddef>
#include <optional>
#include <string>

template<typename K, typename V>
class ICache {
public:
    virtual ~ICache() = default;
    std::optional<V> Get(const K& key) {
        auto result = DoGet(key);
        if (result.has_value()) {
            hits_++;
        } else {
            misses_++;
        }

        return result;
    }

    virtual void Put(K key, V value) = 0;
    virtual std::string GetName() const = 0;

    size_t GetHits() const {
        return hits_;
    }

    size_t GetMisses() const {
        return misses_;
    }

    size_t GetEvictions() const {
        return evictions_;
    }

protected:
    virtual std::optional<V> DoGet(const K& key) = 0;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};

#endif // ICACHE_HPP_
