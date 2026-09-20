#include "subsidiary.hpp"

#include <iostream>
#include <vector>

namespace {
constexpr int kPercentBase = 100;
constexpr int kHotAccessPercent = 80;
constexpr int kHotKeysCount = 10;
constexpr int kFirstKey = 1;
}  // namespace

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
