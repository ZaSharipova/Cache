#include "report.hpp"
#include "experiment.hpp"
#include "command_line/statuses.hpp"

#include <cstddef>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

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
    size_t name_width = std::string("config").size();
    for (const Result& result : results) {
        if (result.seed == seed) {
            name_width = std::max(name_width, result.name.size());
        }
    }
    name_width += 2;

    const size_t levels = MaxLevels(results);

    std::cout << std::left << std::setw(static_cast<int>(name_width)) << "config" << std::right;
    for (size_t i = 1; i <= levels; i++) {
        std::cout << std::setw(8) << ("L" + std::to_string(i));
    }

    std::cout << std::setw(8) << "miss" << std::setw(10) << "hit_ratio" << std::setw(12) << "cost" << "\n";
    std::cout << std::string(name_width + 8 * levels + 30, '-') << "\n";

    for (const Result& result : results) {
        if (result.seed != seed) {
            continue;
        }

        std::cout << std::left << std::setw(static_cast<int>(name_width)) << result.name << std::right;
        for (size_t i = 0; i < levels; i++) {
            std::cout << std::setw(8) << (i < result.hits.size() ? result.hits[i] : 0);
        }
        std::cout << std::setw(8) << result.misses
                  << std::setw(10) << std::fixed << std::setprecision(3) << result.hit_ratio
                  << std::setw(12) << result.cost << "\n";
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

        out << "," << result.misses << "," << result.hit_ratio << ","
            << result.cost << "," << result.seed << "\n";
    }

    out.flush();
    if (!out) {
        std::cerr << "error: failed while writing '" << path << "'\n";
        return Status::kIoError;
    }

    return Status::kOk;
}
