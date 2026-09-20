#include "opt.hpp"

#include <iostream>
#include <vector>
#include <unordered_set>

size_t OPT(const std::vector<int>& trace, size_t cache_size) { // TODO precalculation
    std::unordered_set<int> cache(cache_size);
    size_t hits = 0;

    for (size_t i = 0; i < trace.size(); i++) {
        int key = trace[i];
        if (cache.find(key) != cache.end()) {
            hits++;
            continue;
        }

        if (cache.size() < cache_size) {
            cache.insert(key);

        } else {
            size_t key_to_erase = 0;
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
