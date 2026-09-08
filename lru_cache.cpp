#include <iostream>
#include <list>
#include <optional>

//namespace LRUCache {

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

    std::optional<int> Get(int key) {
        auto it = values_.find(key);
        if (it == values_.end()) {
            misses_++;
            return std::nullopt;
        }

        hits_++;
        MoveToBegin(key);

        return it->second;
    }

    void Put(int key, int value) {
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

     size_t GetHits() const {
         return hits_;
     }

     size_t GetMisses() const {
         return misses_;
     }

     size_t GetEvictions() const {
         return evictions_;
     }
};

class LFUCache {
private:
    size_t capacity_;
    std::unordered_map<int, std::pair<int, size_t>>data_;
    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

public:
    LFUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<int> Get(int key) {
        auto it = data_.find(key);
        if (it == data_.end()) {
            misses_++;
            return std::nullopt;
        }

        hits_++;
        it->second.second++; // как бы появнее это написать
        return it->second.first;
    }

    void Put(int key, int value) {
        auto it = data_.find(key);
        if (it != data_.end()) {
            it->second.first = value;
            it->second.second++;
            return;
        }

        if (data_.size() >= capacity_) {
            size_t min_frequency = SIZE_MAX;
            int victim = 0;
            bool found_flag = false;

            for (auto it : data_) {
                if (it.second.second < min_frequency) {
                    min_frequency = it.second.second;
                    victim = it.first;
                    found_flag = true;
                }

            }

            if (found_flag) {
                data_.erase(victim);
                evictions_++;
            }
        }

        data_[key] = {value, 1};
    }

    size_t GetHits() const {
        return hits_;
    }

    size_t GetMisses() const {
        return misses_;
    }

    size_t GetEvictions() const {
        return evictions_;
    }
};

int main(void) {
    LRUCache cache(2);

    cache.Put(1, 20);
    cache.Put(2, 30);

    std::cout << "Get(1): " << cache.Get(1).value_or(-1) << " Get(2): " << cache.Get(2).value_or(-1) << "\n";

    cache.Put(3, 40);
    auto value = cache.Get(2);
    std::cout << "Get(2): " << (value ? (std::to_string(*value)) : "fail :(") << "\n";

    std::cout << "Get(1): " << cache.Get(1).value_or(-1) << " Get(2): " << cache.Get(2).value_or(-1) << "\n";

    std::cout << "\nHits: " << cache.GetHits()
                << "\nMisses: " << cache.GetMisses()
                << "\nEvictions: " << cache.GetEvictions();

    return 0;
}
//} // namespace LRUCache
