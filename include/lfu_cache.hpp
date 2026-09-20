#ifndef LFU_CACHE_H_
#define LFU_CACHE_H_

#include <iostream>

class LFUCache {

public:
    LFUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

    size_t GetHits() const;
    size_t GetMisses() const;
    size_t GetEvictions() const;

private:
    size_t capacity_;

    struct Entry {
        int value;
        size_t frequency;
    };

    std::unordered_map<int, Entry> data_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};

#endif // LFU_CACHE_H_
