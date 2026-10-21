#ifndef LIRS_CACHE_HPP_
#define LIRS_CACHE_HPP_

#include <iostream>
#include <list>
#include <unordered_map>
#include <optional>

#include "ICache.hpp"

template<typename K, typename V>
class LIRSCache : public ICache<K, V> {
public:
    using key_type = K;
    using value_type = V;

    using ICache<K, V>::hits_;
    using ICache<K, V>::misses_;
    using ICache<K, V>::evictions_;

    LIRSCache(size_t capacity) : capacity_(capacity),
                                 hir_limit_(std::max<size_t>(capacity / kHirDivisor, 1)),
                                 lir_limit_(capacity - hir_limit_) {}

    std::optional<V> Get(const K& key) override {
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
            (entry.in_stack) ? PromoteToLIR(key) : RefreshHIR(key);
        }

        return table_.at(key).value;
    }

    void Put(K key, V value) override {
        auto it = table_.find(key);
        if (it != table_.end()) {

            auto& entry = it->second;
            if (entry.is_resident) {
                entry.value = value;
                if (entry.is_lir) {
                    MoveToStackTop(key);
                } else if (entry.in_stack) {
                    PromoteToLIR(key);
                } else {
                    RefreshHIR(key);
                }

                return;
            }

            PromoteGhostToLIR(key, value);
            return;
        }

        if (lir_count_ < lir_limit_) {
            InsertNewLIR(key, value);
            return;
        }

        InsertNewHIR(key, value);
    }

    std::string GetName() const override {
        return "LIRS";
    }

private:
    static constexpr size_t kHirDivisor = 100;

    struct Entry {
        V value;
        bool is_lir;
        bool is_resident;
        typename std::list<K>::iterator s_it;
        bool in_stack;
        typename std::list<K>::iterator q_it;
        bool in_queue;
    };

    void Prune() {
        while (!stack_s_.empty()) {
            K bottom_key = stack_s_.back();
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

    void MoveToStackTop(K key) {
        Entry& entry = table_.at(key);
        bool was_at_bottom = (entry.s_it == std::prev(stack_s_.end()));
        stack_s_.splice(stack_s_.begin(), stack_s_, entry.s_it);
        entry.s_it = stack_s_.begin();
        if (was_at_bottom) {
            Prune();
        }
    }

    void RemoveFromQueue(Entry& entry) {
        if (entry.in_queue) {
            queue_q_.erase(entry.q_it);
            entry.in_queue = false;
        }
    }

    void AddToQueue(K key, Entry& entry) {
        queue_q_.push_back(key);
        entry.q_it = std::prev(queue_q_.end());
        entry.in_queue = true;
    }

    void PromoteToLIR(K key) {
        Entry& entry = table_.at(key);
        RemoveFromQueue(entry);
        entry.is_lir = true;
        lir_count_++;
        MoveToStackTop(key);
        DemoteLIRFromBottom();
    }

    void RefreshHIR(K key) {
        Entry& entry = table_.at(key);
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

        K bottom_key = stack_s_.back();
        Entry& entry = table_.at(bottom_key);
        stack_s_.pop_back();
        entry.in_stack = false;
        entry.is_lir = false;

        AddToQueue(bottom_key, entry);
        lir_count_--;
        Prune();
    }

    void PromoteGhostToLIR(K key, V value) {
        Entry& entry = table_.at(key);
        if (resident_count_ >= capacity_) {
            EvictFromQueue();
        }

        entry.value = value;
        entry.is_resident = true;
        resident_count_++;
        RemoveFromQueue(entry);

        if (entry.in_stack) {
            entry.is_lir = true;
            MoveToStackTop(key);
            lir_count_++;
            DemoteLIRFromBottom();
        } else {
            stack_s_.push_front(key);
            entry.s_it = stack_s_.begin();
            entry.in_stack = true;
            AddToQueue(key, entry);
            entry.is_lir = false;
        }
    }

    void InsertNewHIR(K key, V value) {
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

    void InsertNewLIR(K key, V value) {
        Entry entry;
        entry.value = value;
        entry.is_lir = true;
        entry.is_resident = true;

        stack_s_.push_front(key);
        entry.s_it = stack_s_.begin();
        entry.in_stack = true;

        table_[key] = entry;
        resident_count_++;
        lir_count_++;
    }

    void EvictFromQueue() {
        if (queue_q_.empty()) {
            return;
        }

        K victim = queue_q_.front();
        queue_q_.pop_front();

        Entry& victim_entry = table_.at(victim);
        victim_entry.is_resident = false;
        victim_entry.in_queue = false;
        resident_count_--;
        evictions_++;

        if (!victim_entry.in_stack) {
            table_.erase(victim);
        }
    }

    const size_t capacity_;
    const size_t hir_limit_;
    const size_t lir_limit_;
    size_t lir_count_ = 0;
    size_t resident_count_ = 0;

    std::list<K> stack_s_;
    std::list<K> queue_q_;

    std::unordered_map<K, Entry> table_;
};

#endif // LIRS_CACHE_HPP_
