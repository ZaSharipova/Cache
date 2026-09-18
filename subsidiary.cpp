#include "subsidiary.hpp"

#include <iostream>
#include <vector>

std::vector<int> MakeHot(int num_keys, int length, unsigned seed) {
    srand(seed);

    std::vector<int> trace;

    trace.reserve(length);
    for (int i = 0; i < length; i++) {
        if (rand() % 100 < 80) {
            trace.push_back(rand() % 10 + 1);
        } else {
            trace.push_back(rand() % num_keys + 1);
        }
    }

    return trace;
}
