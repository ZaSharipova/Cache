#include "report.hpp"
#include "command_line/statuses.hpp"
#include "experiment.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

size_t MaxLevels(const std::vector<Result>& results) {
    size_t max_levels = 0;
    for (const Result& result : results) {
        max_levels = std::max(max_levels, result.hits.size());
    }

    return max_levels;
}

} // namespace

void PrintTable(const std::vector<Result>& results, int seed) {
    constexpr int kLevelWidth = 8;
    constexpr int kMissWidth = 8;
    constexpr int kRatioWidth = 10;
    constexpr int kCostWidth = 12;
    constexpr int kTailWidth = kMissWidth + kRatioWidth + kCostWidth;
    constexpr std::string_view kNameHeader = "config";

    int name_width = static_cast<int>(kNameHeader.size());
    for (const Result& result : results) {
        if (result.seed == seed) {
            name_width = std::max(name_width, static_cast<int>(result.name.size()));
        }
    }
    name_width += 2;

    const size_t levels = MaxLevels(results);
    std::cout << std::left << std::setw(name_width) << kNameHeader << std::right;
    for (size_t i = 1; i <= levels; i++) {
        std::cout << std::setw(kLevelWidth) << ("L" + std::to_string(i));
    }

    std::cout << std::setw(kMissWidth) << "miss" << std::setw(kRatioWidth)
              << "hit_ratio" << std::setw(kCostWidth) << "cost" << "\n";

    const size_t rule_width = static_cast<size_t>(name_width) + static_cast<size_t>(kLevelWidth) * levels + static_cast<size_t>(kTailWidth);
    std::cout << std::string(rule_width, '-') << "\n";
    std::cout << std::fixed << std::setprecision(3);

    for (const Result& result : results) {
        if (result.seed != seed) {
            continue;
        }

        std::cout << std::left << std::setw(name_width) << result.name
                  << std::right;
        for (size_t i = 0; i < levels; i++) {
            std::cout << std::setw(kLevelWidth)
                      << (i < result.hits.size() ? result.hits[i] : 0);
        }
        std::cout << std::setw(kMissWidth) << result.misses
                  << std::setw(kRatioWidth) << result.hit_ratio
                  << std::setw(kCostWidth) << result.cost << "\n";
    }
}

Status WriteCsv(const std::string& path, const std::vector<Result>& results) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "error: cannot open '" << path << "' for writing\n";
        return Status::kIoError;
    }

    const size_t levels = MaxLevels(results);

    out << "config";
    for (size_t i = 1; i <= levels; i++) {
        out << ",h" << i;
    }
    out << ",miss,hit_ratio,cost,seed\n";

    for (const Result& result : results) {
        out << result.name;
        for (size_t i = 0; i < levels; i++) {
            out << "," << (i < result.hits.size() ? result.hits[i] : 0);
        }

        out << "," << result.misses << "," << result.hit_ratio << "," << result.cost
            << "," << result.seed << "\n";
    }

    out.flush();
    if (!out) {
        std::cerr << "error: failed while writing '" << path << "'\n";
        return Status::kIoError;
    }

    return Status::kOk;
}
