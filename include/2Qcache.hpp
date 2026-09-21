#ifndef TWO_Q_CACHE_HPP_
#define TWO_Q_CACHE_HPP_

#include <iostream>
#include <list>
#include <optional>
#include <cassert>
#include <unordered_map>

class TwoQCache {
public:
    TwoQCache(size_t capacity) : a1in_limit_(std::max<size_t>(capacity / kA1inDivisor, 1)),
                                 am_limit_(capacity - a1in_limit_),
                                 a1out_limit_(std::max<size_t>(capacity / kA1outDivisor, 1)) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

    size_t GetHits() const;
    size_t GetMisses() const;
    size_t GetEvictions() const;

private:
    enum class Location {
        kAm,
        kA1in,
        kA1out,
        kNone,
    };

    struct Entry {
        int value;
        Location location;
        std::list<std::pair<int, int>>::iterator data_it;
        std::list<int>::iterator ghost_it;
    };

    void UpdateInAm(Entry &entry, int value);
    void UpdateInA1in(Entry &entry, int value);
    void PromoteFromA1out(int key, Entry &entry, int value);
    void InsertNewKey(int key, int value);

    void EvictFromAm();
    void EvictFromA1in();
    void TrimA1out();

    std::unordered_map<int, Entry> table_;
    const size_t a1in_limit_;
    const size_t am_limit_;
    const size_t a1out_limit_;

    std::list<std::pair<int, int>> a1in_;
    std::list<std::pair<int, int>> am_;
    std::list<int> a1out_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    static constexpr size_t kA1inDivisor = 4;
    static constexpr size_t kA1outDivisor = 2;
};


#endif // TWO_Q_CACHE_HPP_
