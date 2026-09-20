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
    switch(entry.location) {
        case (Location::kAm):
            hits_++;
            am_.splice(am_.begin(), am_, entry.data_it);
            entry.data_it = am_.begin();
            return entry.data_it->second;

        case (Location::kA1in):
            hits_++;
            return entry.data_it->second;

        case (Location::kA1out):
            misses_++;
        default:
            return std::nullopt;
    }
}

void TwoQCache::Put(int key, int value) {
    auto it = table_.find(key);
    if (it == table_.end()) {
        InsertNewKey(key, value);
        return;
    }

    Entry& entry = it->second;
    switch (entry.location) {
        case (Location::kAm):
            UpdateInAm(entry, value);
            break;

        case (Location::kA1in):
            UpdateInA1in(entry, value);
            break;

        case (Location::kA1out): {
            PromoteFromA1out(key, entry, value);
            break;;
        }

        default:
            assert(0);
            std::cerr << "Invalid location\n";
    }
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

void TwoQCache::UpdateInAm(Entry& entry, int value) {
    entry.data_it->second = value;
    am_.splice(am_.begin(), am_, entry.data_it);
    entry.data_it = am_.begin();
}

void TwoQCache::UpdateInA1in(Entry& entry, int value) {
    entry.data_it->second = value;
}

void TwoQCache::PromoteFromA1out(int key, Entry& entry, int value) {
    a1out_.erase(entry.ghost_it);

    if (am_.size() >= am_limit_) {
        EvictFromAm();
    }

    am_.push_front({key, value});
    entry.location = Location::kAm;
    entry.data_it = am_.begin();
    entry.value = value;
}

void TwoQCache::InsertNewKey(int key, int value) {
    if (a1in_.size() >= a1in_limit_) {
        EvictFromA1in();
    }

    a1in_.push_front({key, value});
    Entry entry;
    entry.value = value;
    entry.location = Location::kA1in;
    entry.data_it = a1in_.begin();
    table_[key] = entry;
}

void TwoQCache::EvictFromAm() {
    int victim = am_.back().first;
    am_.pop_back();
    table_.erase(victim);
    evictions_++;
}

void TwoQCache::EvictFromA1in() {
    int old_key = a1in_.back().first;
    a1in_.pop_back();
    evictions_++;

    a1out_.push_front(old_key);
    Entry &old_entry = table_.at(old_key);
    old_entry.location = Location::kA1out;
    old_entry.ghost_it = a1out_.begin();

    TrimA1out();
}

void TwoQCache::TrimA1out() {
    if (a1out_.size() > a1out_limit_) {
        int ghost_key = a1out_.back();
        a1out_.pop_back();
        table_.erase(ghost_key);
    }
}
