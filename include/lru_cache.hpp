#ifndef LRU_CACHE_HPP_
#define LRU_CACHE_HPP_

#include <iostream>
#include <list>

class LRUCache {
public:
    LRUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

     size_t GetHits() const;
     size_t GetMisses() const;
     size_t GetEvictions() const;

private:
    size_t capacity_;
    std::list<std::pair<int, int>> order_;
    std::unordered_map<int, std::list<std::pair<int, int>>::iterator> pos_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};

#endif // LRU_CACHE_HPP_
