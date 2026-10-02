#ifndef OPT_HPP_
#define OPT_HPP_

#include <iostream>
#include <vector>
#include <unordered_set>

template <typename K, typename V>
size_t OPT(const std::vector<K>& trace, size_t cache_size) {
    std::unordered_set<V> cache(cache_size);
    size_t hits = 0;

    for (size_t i = 0; i < trace.size(); i++) {
        K key = trace[i];
        if (cache.find(key) != cache.end()) {
            hits++;
            continue;
        }

        if (cache.size() < cache_size) {
            cache.insert(key);

        } else {
            K key_to_erase = 0;
            size_t farthest_key_pos = 0;
            for (auto key_set : cache) {
                size_t next_pos = trace.size();
                for (size_t j = i + 1; j < trace.size(); j++) {
                    if (trace[j] == key_set) {
                        next_pos = j;
                        break;
                    }
                }

                if (next_pos > farthest_key_pos) {
                    farthest_key_pos = next_pos;
                    key_to_erase = key_set;
                }
            }

            cache.erase(key_to_erase);
            cache.insert(key);
        }
    }

    return hits;
}

#endif // OPT_HPP_
