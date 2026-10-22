#ifndef PARSE_UTILS_HPP_
#define PARSE_UTILS_HPP_

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "statuses.hpp"

namespace Parse {

std::vector<std::string_view> Split(std::string_view text, char delimiter);

std::string ToLower(std::string_view text);

Status ParseSize(std::string_view text, std::string_view what, size_t& out);

Status ParseInt(std::string_view text, std::string_view what, int& out);

} // namespace Parse

#endif // PARSE_UTILS_HPP_
