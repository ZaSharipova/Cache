#ifndef OPT_HPP_
#define OPT_HPP_

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

template <typename K, typename V = void>
size_t OPT(const std::vector<K>& trace, size_t cache_size) {
    std::unordered_set<K> cache(cache_size);
    size_t hits = 0;

    std::size_t i = 0;
    auto next_use = [&trace, &i](const K& key) {
        return std::find(trace.begin() + i + 1, trace.end(), key) - trace.begin();
    };

    for (i = 0; i < trace.size(); i++) {
        K key = trace[i];
        if (cache.find(key) != cache.end()) {
            hits++;
            continue;
        }

        if (cache.size() < cache_size) {
            cache.insert(key);

        } else {
            auto victim = std::max_element(cache.begin(), cache.end(),
                                          [next_use](const K& a, const K& b){
                                              return next_use(a) < next_use(b);
                                          });

            if (victim == cache.end()) {
                continue;
            }

            cache.erase(victim);
            cache.insert(key);
        }
    }

    return hits;
}

#endif // OPT_HPP_
