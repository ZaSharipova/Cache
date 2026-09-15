#include "lirs_cache.hpp"

#include <iostream>

std::optional<int> LIRSCache::Get(int key) {
    auto it = table_.find(key);

    if (it == table_.end()) {
        misses_++;
        return std::nullopt;
    }

    Key& entry = it->second;
    if (!entry.is_resident) {
        misses_++;
        return std::nullopt;
    }

    hits_++;
    if (entry.is_lir) {
        MoveToStackTop(key);
    } else {
        if (entry.in_stack) {
            PromoteToLIR(key);
        } else {
            RefreshHIR(key);
        }
    }

    return table_.at(key).value;
}

void LIRSCache::Put(int key, int value) {
    auto it = table_.find(key);
    if (it != table_.end() && it->second.is_resident) {
        it->second.value = value;
        if (it->second.is_lir) {
            MoveToStackTop(key);
        } else if (it->second.in_stack) {
            PromoteToLIR(key);
        } else {
            RefreshHIR(key);
        }
        return;
    }

    if (it != table_.end() && !it->second.is_resident) {
        PromoteGhostToLIR(key, value);
        return;
    }

    InsertNewHIR(key, value);
}

size_t LIRSCache::GetHits() const {
    return hits_;
}

size_t LIRSCache::GetMisses() const {
    return misses_;
}

size_t LIRSCache::GetEvictions() const {
    return evictions_;
}
