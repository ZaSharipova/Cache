#ifndef ICACHE_HPP_
#define ICACHE_HPP_

#include <cstddef>
#include <optional>
#include <string>

template<typename K, typename V>
class ICache {
public:
    using key_type = K;
    using value_type = V;

    virtual ~ICache() = default;
    virtual std::optional<V> Get(const K& key) = 0;
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
    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};

#endif // ICACHE_HPP_
