#ifndef ARC_CACHE_HPP_
#define ARC_CACHE_HPP_

#include <cstdint>
#include <iostream>
#include <list>
#include <unordered_map>

enum class Location {
    kT1,
    kT2,
    kB1,
    kB2
};

class ARCCache {
public:
    ARCCache(size_t capacity) : capacity_(capacity) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

    size_t GetHits() const;
    size_t GetMisses() const;
    size_t GetEvictions() const;

private:
    size_t capacity_;

    struct Entry {
        int key;
        int value;
    };

    struct Node {
        Location location;
        std::list<Entry> data_it;
        std::list<int> ghost_it;
    };

    std::list<Entry> t1_, t2_;
    std::list<int_fast16_t> b1_, b2_;
    std::unordered_map<int, Node> table_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    void MoveToT2Front(Node& node);
};

#endif // ARC_CACHE_HPP_
