#ifndef ARC_CACHE_HPP_
#define ARC_CACHE_HPP_

#include <cstdint>
#include <iostream>
#include <list>
#include <unordered_map>

template <typename K, typename V>
class ARCCache {
public:
    using key_type = K;
    using value_type = V;

    ARCCache(size_t capacity) : capacity_(capacity) {}

    std::optional<V> Get(K key) {
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

    void Put(K key, V value) {
        if (capacity_ == 0) { // не знаю, насколько правильное это решение
            return;
        }

        auto it = table_.find(key);
        if (it == table_.end()) {
            InsertNewKey(key, value);
            return;
        }

        Node& node = it->second;
        switch (node.location) {
            case (Location::kT1):
            case (Location::kT2):
                node.data_it->value = value;
                MoveToT2Front(node);
                break;

            case (Location::kB1):
            case (Location::kB2):
                PromoteFromGhost(key, node, value);
                break;
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

    size_t Size() const {
        return t1_.size() + t2_.size();
    }

    size_t CheckInvariants() const {
        const size_t t1_b1_size = t1_.size() + b1_.size();
        const size_t total_size = t1_b1_size + t2_.size() + b2_.size();

        return Size() <= capacity_ &&
               t1_b1_size <= capacity_ &&
               total_size <= 2 * capacity_ &&
               t1_target_size_ <= capacity_ &&
               table_.size() == total_size;
    }

private:
    size_t capacity_;
    size_t t1_target_size_ = 0;

    struct Entry {
        K key;
        V value;
    };

    enum class Location {
        kT1,
        kT2,
        kB1,
        kB2
    };

    struct Node {
        Location location;
        typename std::list<Entry>::iterator data_it;
        typename std::list<K>::iterator ghost_it;
    };

    std::list<Entry> t1_, t2_;
    std::list<K> b1_, b2_;
    std::unordered_map<K, Node> table_;

    size_t hits_ = 0, misses_ = 0, evictions_ = 0;

    void MoveToT2Front(Node& node) {
        if (node.location == Location::kT1) {
            t2_.splice(t2_.begin(), t1_, node.data_it);
            node.location = Location::kT2;
        } else {
            t2_.splice(t2_.begin(), t2_, node.data_it);
        }
    }

    void PromoteFromGhost(K key, Node& node, V value) {
        if (node.location == Location::kB2) {
            size_t difference = std::max<size_t>(b1_.size() / b2_.size(), 1);
            t1_target_size_ = (t1_target_size_ > difference) ? t1_target_size_ - difference : 0;
            b2_.erase(node.ghost_it);
        } else {
            size_t difference = std::max<size_t>(b2_.size() / b1_.size(), 1);
            t1_target_size_ = std::min(capacity_, t1_target_size_ + difference);
            b1_.erase(node.ghost_it);
        }

        Replace(node.location == Location::kB2);

        t2_.push_front({key, value});
        node.location = Location::kT2;
        node.data_it = t2_.begin();
    }

    void InsertNewKey(K key, V value) {
        const size_t t1_b1_size = t1_.size() + b1_.size();
        const size_t total_size = t1_b1_size + t2_.size() + b2_.size();

        if (t1_b1_size == capacity_) {
            if (t1_.size() < capacity_) {
                DropGhostLru(b1_);
                Replace(false);
            } else {
                K victim = t1_.back().key;
                t1_.pop_back();
                table_.erase(victim);
                evictions_++;
            }

        } else if (total_size >= capacity_) {
            if (total_size >= 2 * capacity_) {
                DropGhostLru(b2_);
            }

            Replace(false);
        }

        t1_.push_front({key, value});
        table_.emplace(key, Node{Location::kT1, t1_.begin(), {}});
    }

    void Replace(bool key_in_b2) {
        bool replace_from_t1 = !t1_.empty() && (t1_.size() > t1_target_size_ || (key_in_b2 && t1_.size() == t1_target_size_));
        if (t2_.empty()) {
            replace_from_t1 = true;
        }

        if (replace_from_t1) {
            K victim = t1_.back().key;
            t1_.pop_back();
            b1_.push_front(victim);
            Node& node = table_.at(victim);
            node.location = Location::kB1;
            node.ghost_it = b1_.begin();
        } else {
            K victim = t2_.back().key;
            t2_.pop_back();
            b2_.push_front(victim);
            Node& node = table_.at(victim);
            node.location = Location::kB2;
            node.ghost_it = b2_.begin();
        }

        evictions_++;
    }

    void DropGhostLru(std::list<K>& ghost) {
        table_.erase(ghost.back());
        ghost.pop_back();
    }
};

#endif // ARC_CACHE_HPP_
