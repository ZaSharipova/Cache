#include "subsidiary.hpp"

#include <iostream>
#include <vector>

#include "subsidiary.hpp"

#include <cstdlib>

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
    srand(seed);

    std::vector<int> trace;

    trace.reserve(length);
    int scan_key = kMixedScanFirstKey;
    for (int i = 0; i < length; i++) {
        if (rand() % kMixedHotChanceOutOf == 0) {
            trace.push_back(rand() % kMixedHotKeysCount + kFirstKey);
        } else {
            trace.push_back(scan_key++);
            if (scan_key > kMixedScanLastKey) scan_key = kMixedScanFirstKey;
        }
    }

    return trace;
}

std::vector<int> MakeHot(int num_keys, int length, unsigned seed) {
    srand(seed);

    std::vector<int> trace;

    trace.reserve(length);
    for (int i = 0; i < length; i++) {
        if (rand() % kPercentBase < kHotAccessPercent) {
            trace.push_back(rand() % kHotKeysCount + kFirstKey);
        } else {
            trace.push_back(rand() % num_keys + kFirstKey);
        }
    }

    return trace;
}

std::vector<int> MakeRandomTrace(size_t length, int key_range, unsigned seed) {
    srand(seed);
    std::vector<int> trace;

    trace.reserve(length);
    for (size_t i = 0; i < length; i++) {
        trace.push_back(rand() % key_range + 1);
    }

    return trace;
}
