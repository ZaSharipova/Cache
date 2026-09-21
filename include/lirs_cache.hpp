#ifndef LIRS_CACHE_HPP_
#define LIRS_CACHE_HPP_

#include <iostream>
#include <list>
#include <unordered_map>
#include <optional>

class LIRSCache {
public:
    LIRSCache(size_t capacity) : capacity_(capacity),
                                 hir_limit_(std::max<size_t>(capacity / kHirDivisor, 1)),
                                 lir_limit_(capacity - hir_limit_) {}

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

    struct Entry {
        int value;
        bool is_lir;
        bool is_resident;
        std::list<int>::iterator s_it;
        bool in_stack;
        std::list<int>::iterator q_it;
        bool in_queue;
    };

    std::unordered_map<int, Entry> table_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    static constexpr size_t kHirDivisor = 100;

    void Prune();
    void MoveToStackTop(int key);
    void RemoveFromQueue(Entry& entry);
    void AddToQueue(int key, Entry& entry);
    void PromoteToLIR(int key);
    void RefreshHIR(int key);
    void DemoteLIRFromBottom();
    void PromoteGhostToLIR(int key, int value);
    void InsertNewHIR(int key, int value);
    void EvictFromQueue();
};

#endif // LIRS_CACHE_HPP_
