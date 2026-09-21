#include "arc_cache.hpp"

#include <iostream>
#include <optional>

std::optional<int> ARCCache::Get(int key, int value) {
    auto it = table_.find(key);
    if (it == table_.end()) {
        misses_++;
        return std::nullopt;
    }

    Node& node = it->second;
    switch (node.location) {
        case (Location::kT1):
        case (Location::kT2):
            hits_++;
            MoveToT2Front(node);
            return node.data_it->value;

        case (Location::kB1):
        case (Location::kB2):
            misses_++;
            return std::nullopt;

        default:
            return std::nullopt;
    }
}

void ARCCache::Put(int key, int value) {
    auto it = table_.find(key);
    if (it == table_.end()) {
        InsertNewKey(key, value);
        return;
    }

    Node& node = it->second;
    switch (node.location) {

    }
}

size_t ARCCache::GetHits() const {
    return hits_;
}

size_t ARCCache::GetMisses() const {
    return misses_;
}

size_t ARCCache::GetEvictions() const {
    return evictions_;
}

void ARCCache::MoveToT2Front(Node& node) {
    if (node.location == Location::kT1) {
        t2_.splice(t2_.begin, t1_, node.data_it);
        node.location = Location::kT2;
        node.data_it = t2_.begin();
    } else {
        t2_.splice(t2_.begin, t2_, node.data_it);
        node.data_it = t2_.begin();
    }
}
