#include "subsidiary.hpp"

#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include "subsidiary.hpp"

namespace {
constexpr int kFirstKey = 1;

constexpr int kPercentBase = 100;
constexpr int kHotAccessPercent = 80;
constexpr int kHotKeysCount = 10;

constexpr int kMixedHotKeysCount = 5;
constexpr int kMixedScanFirstKey = 100;
constexpr int kMixedScanLastKey = 999;
constexpr int kMixedHotChanceOutOf = 2;

} // namespace

std::vector<int> MakeScan(int num_keys, int length) {
    std::vector<int> trace;

    trace.reserve(length);
    for (int i = 0; i < length; i++) {
        trace.push_back(i % num_keys + kFirstKey);
    }

    return trace;
}

std::vector<int> MakeMixed(int length, unsigned seed) {
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> hot_chance(0, kMixedHotChanceOutOf - 1);
    std::uniform_int_distribution<int> hot_key(
        kFirstKey, kFirstKey + kMixedHotKeysCount - 1);

    std::vector<int> trace;
    trace.reserve(length);

    int scan_key = kMixedScanFirstKey;
    for (int i = 0; i < length; i++) {
        if (hot_chance(gen) == 0) {
            trace.push_back(hot_key(gen));
        } else {
            trace.push_back(scan_key++);
            if (scan_key > kMixedScanLastKey) {
                scan_key = kMixedScanFirstKey;
            }
        }
    }

    return trace;
}

std::vector<int> MakeHot(int num_keys, int length, unsigned seed) {
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> percent(0, kPercentBase - 1);
    std::uniform_int_distribution<int> hot_key(kFirstKey,
        kFirstKey + kHotKeysCount - 1);
    std::uniform_int_distribution<int> any_key(kFirstKey,
        kFirstKey + num_keys - 1);

    std::vector<int> trace;
    trace.reserve(length);

    for (int i = 0; i < length; i++) {
        bool is_hot_key = percent(gen) < kHotAccessPercent;
        trace.push_back(is_hot_key ? hot_key(gen) : any_key(gen));
    }

    return trace;
}

std::vector<int> MakeRandomTrace(size_t length, int key_range, unsigned seed) {
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> key(1, key_range);

    std::vector<int> trace;
    trace.reserve(length);

    for (size_t i = 0; i < length; i++) {
        trace.push_back(key(gen));
    }

    return trace;
}

std::vector<int> MakePhaseShift(int length, unsigned seed) {
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> percent(0, kPercentBase - 1);
    std::uniform_int_distribution<int> hot_offset(0, 9);
    std::uniform_int_distribution<int> any_key(1, 400);

    std::vector<int> trace;
    trace.reserve(length);

    for (int i = 0; i < length; i++) {
        bool first_phase = (i < length / 2);
        int hot_low = first_phase ? 1 : 200;
        bool is_hot_key = percent(gen) < kHotAccessPercent;
        trace.push_back(is_hot_key ? hot_low + hot_offset(gen) : any_key(gen));
    }

    return trace;
}
