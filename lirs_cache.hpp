#ifndef LIRS_CACHE_H_
#define LIRS_CACHE_H_

#include <iostream>
#include <list>
#include <stack>
#include <unordered_map>
#include <optional>

const size_t MAGIC_CAPACITY_DIVISOR = 100;

class LIRSCache {
public:
    LIRSCache(size_t capacity) : capacity_(capacity),
                                 hir_limit_(capacity / MAGIC_CAPACITY_DIVISOR > 0 ? capacity / MAGIC_CAPACITY_DIVISOR : 1),
                                 lir_limit_(capacity - (capacity / MAGIC_CAPACITY_DIVISOR > 0 ? capacity / MAGIC_CAPACITY_DIVISOR : 1)) {}

    std::optional<int> Get(int key);
    void Put(int key, int value);

    size_t GetHits() const;
    size_t GetMisses() const;
    size_t GetEvictions() const;

private:
    const size_t capacity_;
    const size_t hir_limit_;
    const size_t lir_limit_;
    size_t lir_count_ = 0;
    size_t resident_count_ = 0;

    std::list<int> stack_s_;
    std::list<int> queue_q_;

    struct Key {
        int value;
        bool is_lir;
        bool is_resident;
        std::list<int>::iterator s_it;
        bool in_stack;
        std::list<int>::iterator q_it;
        bool in_queue;
    };

    std::unordered_map<int, Key> table_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    void Prune() {
        while (!stack_s_.empty()) {
            int bottom_key = stack_s_.back();
            Key& entry = table_.find(bottom_key)->second;

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

    void MoveToStackTop(int key) {
        Key& entry = table_.at(key);
        bool was_at_bottom = (entry.s_it == std::prev(stack_s_.end()));
        stack_s_.splice(stack_s_.begin(), stack_s_, entry.s_it);
        entry.s_it = stack_s_.begin();
        if (was_at_bottom) {
            Prune();
        }
    }

    void RemoveFromQueue(Key& entry) {
        if (entry.in_queue) {
            queue_q_.erase(entry.q_it);
            entry.in_queue = false;
        }
    }

    void AddToQueue(int key, Key& entry) {
        queue_q_.push_back(key);
        entry.q_it = std::prev(queue_q_.end());
        entry.in_queue = true;
    }

    void PromoteToLIR(int key) {
        Key& entry = table_.at(key);
        RemoveFromQueue(entry);
        entry.is_lir = true;
        lir_count_++;
        MoveToStackTop(key);
        DemoteLIRFromBottom();
    }

    void RefreshHIR(int key) {
        Key& entry = table_.at(key);
        stack_s_.push_front(key);
        entry.s_it = stack_s_.begin();
        entry.in_stack = true;

        RemoveFromQueue(entry);
        AddToQueue(key, entry);
    }

    void DemoteLIRFromBottom() {
        if (lir_count_ <= lir_limit_) {
            return;
        }

        Prune();
        if (stack_s_.empty()) {
            return;
        }

        int bottom_key = stack_s_.back();
        Key& entry = table_.at(bottom_key);
        stack_s_.pop_back();
        entry.in_stack = false;
        entry.is_lir = false;

        AddToQueue(bottom_key, entry);
        lir_count_--;
        Prune();
    }

    void PromoteGhostToLIR(int key, int value) {
        Key& entry = table_.at(key);
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

    void InsertNewHIR(int key, int value) {
        if (resident_count_ >= capacity_) {
            EvictFromQueue();
        }

        Key entry;
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

    void EvictFromQueue() {
        if (queue_q_.empty()) {
            return;
        }

        int victim = queue_q_.front();
        queue_q_.pop_front();

        Key& victim_entry = table_.at(victim);
        victim_entry.is_resident = false;
        victim_entry.in_queue = false;
        resident_count_--;
        evictions_++;
    }
};

#endif // LIRS_CACHE_H_
