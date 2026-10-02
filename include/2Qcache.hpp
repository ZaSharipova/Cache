#ifndef TWO_Q_CACHE_HPP_
#define TWO_Q_CACHE_HPP_

#include <iostream>
#include <list>
#include <optional>
#include <cassert>
#include <unordered_map>

template<typename K, typename V>
class TwoQCache {
public:
    using key_type = K;
    using value_type = V;

    TwoQCache(size_t capacity) : a1in_limit_(std::max<size_t>(capacity / kA1inDivisor, 1)),
                                 am_limit_(capacity - a1in_limit_),
                                 a1out_limit_(std::max<size_t>(capacity / kA1outDivisor, 1)) {}

    std::optional<V> Get(K key) {
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

    void Put(K key, V value) {
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

    size_t GetHits() const {
        return hits_;
    }

    size_t GetMisses() const {
        return misses_;
    }

    size_t GetEvictions() const {
        return evictions_;
    }

private:

    enum class Location {
        kAm,
        kA1in,
        kA1out,
        kNone,
    };

    struct Entry {
        V value;
        Location location;
        typename std::list<std::pair<K, V>>::iterator data_it;
        typename std::list<K>::iterator ghost_it;
    };

    void UpdateInAm(Entry& entry, V value) {
        entry.data_it->second = value;
        am_.splice(am_.begin(), am_, entry.data_it);
        entry.data_it = am_.begin();
    }

    void UpdateInA1in(Entry& entry, V value) {
        entry.data_it->second = value;
    }

    void PromoteFromA1out(K key, Entry& entry, V value) {
        a1out_.erase(entry.ghost_it);

        if (am_.size() >= am_limit_) {
            EvictFromAm();
        }

        am_.push_front({key, value});
        entry.location = Location::kAm;
        entry.data_it = am_.begin();
        entry.value = value;
    }

    void InsertNewKey(K key, V value) {
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

    void EvictFromAm() {
        K victim = am_.back().first;
        am_.pop_back();
        table_.erase(victim);
        evictions_++;
    }

    void EvictFromA1in() {
        K old_key = a1in_.back().first;
        a1in_.pop_back();
        evictions_++;

        a1out_.push_front(old_key);
        Entry &old_entry = table_.at(old_key);
        old_entry.location = Location::kA1out;
        old_entry.ghost_it = a1out_.begin();

        TrimA1out();
    }

    void TrimA1out() {
        if (a1out_.size() > a1out_limit_) {
            K ghost_key = a1out_.back();
            a1out_.pop_back();
            table_.erase(ghost_key);
        }
    }

    static constexpr size_t kA1inDivisor = 4;
    static constexpr size_t kA1outDivisor = 2;

    std::unordered_map<K, Entry> table_;
    const size_t a1in_limit_;
    const size_t am_limit_;
    const size_t a1out_limit_;

    std::list<std::pair<K, V>> a1in_;
    std::list<std::pair<K, V>> am_;
    std::list<K> a1out_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;
};

#endif // TWO_Q_CACHE_HPP_
