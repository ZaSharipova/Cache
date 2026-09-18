#include "2Qcache.hpp"

#include <iostream>
#include <list>
#include <optional>
#include <cassert>

std::optional<int> TwoQCache::Get(int key) {
    auto it = table_.find(key);
    if (it == table_.end()) {
        misses_++;
        return std::nullopt;
    }

    Entry& entry = it->second;

    if (entry.location == Location::kAm) {
        hits_++;
        am_.splice(am_.begin(), am_, entry.data_it);
        entry.data_it = am_.begin();
        return entry.data_it->second;
    }

    if (entry.location == Location::kA1in) {
        hits_++;
        return entry.data_it->second;
    }

    misses_++;
    return std::nullopt;
}

void TwoQCache::Put(int key, int value) {
    auto it = table_.find(key);
    if (it != table_.end()) {
        switch (it->second.location) {
            case Location::kAm:
                it->second.data_it->second = value;
                am_.splice(am_.begin(), am_, it->second.data_it);
                it->second.data_it = am_.begin();
                return;

            case Location::kA1in:
                it->second.data_it->second = value;
                return;

            case Location::kA1out: {
                Entry &entry = it->second;
                a1out_.erase(entry.ghost_it);

                if (am_.size() >= am_limit_) {
                    int victim = am_.back().first;
                    am_.pop_back();
                    table_.erase(victim);
                    evictions_++;
                }

                am_.push_front({key, value});
                entry.location = Location::kAm;
                entry.data_it = am_.begin();
                entry.value = value;
                return;
            }

            default:
                return; // TODO
        }
    }

    if (a1in_.size() >= a1in_limit_) {
        int old_key = a1in_.back().first;
        a1in_.pop_back();
        evictions_++;

        a1out_.push_front(old_key);
        Entry &old_entry = table_.at(old_key);
        old_entry.location = Location::kA1out;
        old_entry.ghost_it = a1out_.begin();

        if (a1out_.size() > a1out_limit_) {
            int ghost_key = a1out_.back();
            a1out_.pop_back();
            table_.erase(ghost_key);
        }
    }

    a1in_.push_front({key, value});
    Entry entry;
    entry.value = value;
    entry.location = Location::kA1in;
    entry.data_it = a1in_.begin();
    table_[key] = entry;
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
