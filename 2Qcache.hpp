#ifndef TWO_Q_CACHE_H_
#define TWO_Q_CACHE_H_

#include <iostream>
#include <list>
#include <optional>
#include <cassert>

class TwoQCache {
public:
    TwoQCache(size_t capacity) : a1in_limit_(capacity / 4),
                                 am_limit_(capacity - capacity / 4),
                                 a1out_limit_(capacity / 2) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

    size_t GetHits() const;
    size_t GetMisses() const;
    size_t GetEvictions() const;

private:
    //const size_t capacity_;
    const size_t a1in_limit_;
    const size_t am_limit_;
    const size_t a1out_limit_;

    std::list<std::pair<int, int>> a1in_;
    std::list<std::pair<int, int>> am_;
    std::list<int> a1out_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};


#endif // TWO_Q_CACHE_H_
