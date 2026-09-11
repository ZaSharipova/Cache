#ifndef LRU_CACHE_H_
#define LRU_CACHE_H_

#include <iostream>
#include <list>

class LRUCache {
private:
    size_t capacity_;
    std::list<int> order_; // ключи в порядке использования
    std::unordered_map<int, int> values_; // ключ -> значение

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    void MoveToBegin(int key) {
        order_.remove(key);
        order_.push_front(key);
    }

public:
    LRUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

     size_t GetHits() const;
     size_t GetMisses() const;
     size_t GetEvictions() const;
};

#endif // LRU_CACHE_H_
