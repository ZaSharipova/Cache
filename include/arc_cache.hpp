#ifndef ARC_CACHE_HPP_
#define ARC_CACHE_HPP_

#include <cstdint>
#include <iostream>
#include <list>
#include <unordered_map>

class ARCCache {
public:
    ARCCache(size_t capacity) : capacity_(capacity) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

    size_t GetHits() const;
    size_t GetMisses() const;
    size_t GetEvictions() const;

    size_t Size() const;
    size_t CheckInvariants() const;

private:
    size_t capacity_;
    size_t t1_target_size_ = 0;

    struct Entry {
        int key;
        int value;
    };

    enum class Location {
        kT1,
        kT2,
        kB1,
        kB2
    };

    struct Node {
        Location location;
        std::list<Entry>::iterator data_it;
        std::list<int>::iterator ghost_it;
    };

    std::list<Entry> t1_, t2_;
    std::list<int> b1_, b2_;
    std::unordered_map<int, Node> table_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    void MoveToT2Front(Node& node);
    void PromoteFromGhost(int key, Node& node, int value);
    void InsertNewKey(int key, int value);

    void Replace(bool key_in_b2);
    void DropGhostLru(std::list<int>& ghost);
};

#endif // ARC_CACHE_HPP_
