#include "lru_cache.hpp"

#include <iostream>
#include <list>
#include <optional>

std::optional<int> LRUCache::Get(int key) {
    auto it = values_.find(key);
    if (it == values_.end()) {
        misses_++;
        return std::nullopt;
    }

    hits_++;
    MoveToBegin(key);

    return it->second;
}

void LRUCache::Put(int key, int value) {
    auto it = values_.find(key);
    if (it != values_.end()) {
        it->second = value;
        MoveToBegin(key);
        return;
    }

    if (values_.size() >= capacity_) {
        int victim = order_.back();
        order_.pop_back();
        values_.erase(victim);

        evictions_++;
    }

    order_.push_front(key);
    values_[key] = value;
}

size_t LRUCache::GetHits() const {
    return hits_;
}

size_t LRUCache::GetMisses() const {
    return misses_;
}

size_t LRUCache::GetEvictions() const {
    return evictions_;
}


// std::vector<int> MakeScan(int num_keys, int length) {
//     std::vector<int> trace;

//     for (int i = 0; i < length; i++) {
//         trace.push_back(i % num_keys + 1);
//     }

//     return trace;
// }

// std::vector<int> MakeHot(int num_keys, int length) {
//     std::vector<int> trace;
//     for (int i = 0; i < length; i++) {
//         int random_number = rand() % 100;
//         if (random_number < 80) {
//             trace.push_back(rand() % 10 + 1); // а тут уменьшаем интервал до [1, 10]
//         } else {
//             trace.push_back(rand() % num_keys + 1); // тут мы берем любой ключ, оставляем нормальное распределение
//         }
//     }

//     return trace;
// }

// template <typename CacheT>
// double Run(CacheT& cache, const std::vector<int>& trace) {
//     for (size_t i = 0; i < trace.size(); i++) {
//         int key = trace[i];
//         auto value = cache.Get(key);
//         if (!value) {
//             cache.Put(key, key * 10); // чтобы хоть что-то положить, потом придумаю как заменить magic numbers TODO
//         }
//     }

//     double total = cache.GetHits() + cache.GetMisses();
//     return cache.GetHits() / total;
// }

// int main(void) {
//     srand(42);

//     int capacity = 10;
//     std::vector<int> hot_numbers = MakeHot(DEFAULT_CAPACITY, 10000);
//     std::vector<int> scan_numbers = MakeScan(20, 10000);

//     std::cout << "Cache capacity: " << capacity << "\n";

//     std::cout << "\nHOT CHECK\n";
//     LRUCache lru1(capacity);
//     std::cout << "LRU hit ratio: " << Run<LRUCache>(lru1, hot_numbers) << "\n";
//     LFUCache lfu1(capacity);
//     std::cout << "LFU hit ratio: " << Run<LFUCache>(lfu1, hot_numbers) << "\n";

//     std::cout << "\n\nSCAN CHECK\n";
//     LRUCache lru2(capacity);
//     std::cout << "LRU hit ratio: " << Run<LRUCache>(lru2, scan_numbers) << "\n";
//     LFUCache lfu2(capacity);
//     std::cout << "LFU hit ratio: " << Run<LFUCache>(lfu2, scan_numbers) << "\n";

    // LRUCache cache(2);

    // cache.Put(1, 20);
    // cache.Put(2, 30);

    // std::cout << "Get(1): " << cache.Get(1).value_or(-1) << " Get(2): " << cache.Get(2).value_or(-1) << "\n";

    // cache.Put(3, 40);
    // auto value = cache.Get(2);
    // std::cout << "Get(2): " << (value ? (std::to_string(*value)) : "fail :(") << "\n";

    // std::cout << "Get(1): " << cache.Get(1).value_or(-1) << " Get(2): " << cache.Get(2).value_or(-1) << "\n";

    // std::cout << "\nHits: " << cache.GetHits()
    //             << "\nMisses: " << cache.GetMisses()
    //             << "\nEvictions: " << cache.GetEvictions();

//     return 0;
// }
