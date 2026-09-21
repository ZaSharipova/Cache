#ifndef SUBSIDIARY_HPP_
#define SUBSIDIARY_HPP_

#include <vector>

std::vector<int> MakeScan(int num_keys, int length);
std::vector<int> MakeMixed(int length, unsigned seed);
std::vector<int> MakeHot(int num_keys, int length, unsigned seed);
std::vector<int> MakeRandomTrace(size_t length, int key_range, unsigned seed);

#endif // SUBSIDIARY_HPP_
