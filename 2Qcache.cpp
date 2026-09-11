#include "2Qcache.hpp"

#include <iostream>
#include <list>
#include <optional>
#include <cassert>

std::optional<int> TwoQCache::Get(int key) {
    for (auto it = am_.begin(); it != am_.end(); it++) {
        if (it->first == key) {
            hits_++;
            int value = it->second;
            am_.splice(am_.begin(), am_, it);
            return value;
        }
    }

    for (auto it = a1in_.begin(); it != a1in_.end(); it++) {
        if (it->first == key) {
            hits_++;
            return it->second;
        }
    }

    misses_++;
    return std::nullopt;
}

void TwoQCache::Put(int key, int value) {
    for (auto it = am_.begin(); it != am_.end(); it++) {
        if (it->first == key) {
            it->second = value;
            am_.splice(am_.begin(), am_, it);
            return;
        }
    }

    for (auto it = a1in_.begin(); it != a1in_.end(); it++) {
        if (it->first == key) {
            it->second = value;
            return;
        }
    }

    for (auto it = a1out_.begin(); it != a1out_.end(); it++) {
        if (*it == key) {
            a1out_.erase(it);

            if (am_.size() >= am_limit_) {
                am_.pop_back();
                evictions_++;
            }

            am_.push_front({key, value});
            return;
        }
    }

    if (a1in_.size() >= a1in_limit_) {
        int old_key = a1in_.back().first;
        a1in_.pop_back();
        a1out_.push_front(old_key);

        if (a1out_.size() > a1out_limit_) {
            a1out_.pop_back();
            evictions_++; // а считается ли оно таковым
        }
    }

    a1in_.push_front({key, value});
}

size_t TwoQCache::GetHits() const {
    return hits_;
}

size_t TwoQCache::GetMisses() const {
    return misses_;
}

size_t TwoQCache::GetEvictions() const {
    return evictions_;
}
