#ifndef REPORT_HPP_
#define REPORT_HPP_

#include "command_line/statuses.hpp"
#include "experiment.hpp"

#include <cstddef>
#include <string>
#include <vector>

void PrintTable(const std::vector<Result>& results, int seed);
Status WriteCsv(const std::string& path, const std::vector<Result>& results);

#endif // REPORT_HPP_
