#ifndef EXPERIMENT_HPP_
#define EXPERIMENT_HPP_

#include "command_line/cache_factory.hpp"
#include "command_line/command_line_interface.hpp"
#include "command_line/statuses.hpp"
#include "subsidiary.hpp"

#include <cstddef>
#include <string>
#include <vector>

struct Result {
    std::string name;
    std::vector<size_t> hits;
    size_t misses = 0;
    double hit_ratio = 0.0;
    size_t cost = 0;
    int seed = 0;
};

Status RunSetup(const CacheFactory::Setup& setup, const std::vector<size_t>& costs,
    const std::vector<int>& trace, int seed, Result& result);
Result RunOpt(const std::vector<int>& trace, size_t capacity, int seed);
Status RunExperiment(const CLI::Config& config, std::vector<Result>& results);

#endif // EXPERIMENT_HPP_
