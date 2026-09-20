#include "lirs_cache.hpp"

#include <iostream>

std::optional<int> LIRSCache::Get(int key) {
    auto it = table_.find(key);

    if (it == table_.end()) {
        misses_++;
        return std::nullopt;
    }

    Entry& entry = it->second;
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

void LIRSCache::Prune() {
    while (!stack_s_.empty()) {
        int bottom_key = stack_s_.back();
        Entry& entry = table_.find(bottom_key)->second;

        if (entry.is_lir) {
            break;
        }

        stack_s_.pop_back();
        entry.in_stack = false;

        if (!entry.is_resident) {
            table_.erase(bottom_key);
        }
    }
}

void LIRSCache::MoveToStackTop(int key) {
    Entry& entry = table_.at(key);
    bool was_at_bottom = (entry.s_it == std::prev(stack_s_.end()));
    stack_s_.splice(stack_s_.begin(), stack_s_, entry.s_it);
    entry.s_it = stack_s_.begin();
    if (was_at_bottom) {
        Prune();
    }
}

void LIRSCache::RemoveFromQueue(Entry& entry) {
    if (entry.in_queue) {
        queue_q_.erase(entry.q_it);
        entry.in_queue = false;
    }
}

void LIRSCache::AddToQueue(int key, Entry& entry) {
    queue_q_.push_back(key);
    entry.q_it = std::prev(queue_q_.end());
    entry.in_queue = true;
}

void LIRSCache::PromoteToLIR(int key) {
    Entry& entry = table_.at(key);
    RemoveFromQueue(entry);
    entry.is_lir = true;
    lir_count_++;
    MoveToStackTop(key);
    DemoteLIRFromBottom();
}

void LIRSCache::RefreshHIR(int key) {
    Entry& entry = table_.at(key);
    stack_s_.push_front(key);
    entry.s_it = stack_s_.begin();
    entry.in_stack = true;

    RemoveFromQueue(entry);
    AddToQueue(key, entry);
}

void LIRSCache::DemoteLIRFromBottom() {
    if (lir_count_ <= lir_limit_) {
        return;
    }

    Prune();
    if (stack_s_.empty()) {
        return;
    }

    int bottom_key = stack_s_.back();
    Entry& entry = table_.at(bottom_key);
    stack_s_.pop_back();
    entry.in_stack = false;
    entry.is_lir = false;

    AddToQueue(bottom_key, entry);
    lir_count_--;
    Prune();
}

void LIRSCache::PromoteGhostToLIR(int key, int value) {
    Entry& entry = table_.at(key);
    if (resident_count_ >= capacity_) {
        EvictFromQueue();
    }

    entry.value = value;
    entry.is_resident = true;
    resident_count_++;
    RemoveFromQueue(entry);

    entry.is_lir = true;
    if (entry.in_stack) {
        MoveToStackTop(key);
    } else {
        stack_s_.push_front(key);
        entry.s_it = stack_s_.begin();
        entry.in_stack = true;
    }

    lir_count_++;
    DemoteLIRFromBottom();
}

void LIRSCache::InsertNewHIR(int key, int value) {
    if (resident_count_ >= capacity_) {
        EvictFromQueue();
    }

    Entry entry;
    entry.value = value;
    entry.is_lir = false;
    entry.is_resident = true;

    stack_s_.push_front(key);
    entry.s_it = stack_s_.begin();
    entry.in_stack = true;

    queue_q_.push_back(key);
    entry.q_it = std::prev(queue_q_.end());
    entry.in_queue = true;

    table_[key] = entry;
    resident_count_++;
}

void LIRSCache::EvictFromQueue() {
    if (queue_q_.empty()) {
        return;
    }

    int victim = queue_q_.front();
    queue_q_.pop_front();

    Entry& victim_entry = table_.at(victim);
    victim_entry.is_resident = false;
    victim_entry.in_queue = false;
    resident_count_--;
    evictions_++;
}
