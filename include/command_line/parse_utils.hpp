#ifndef PARSE_UTILS_HPP_
#define PARSE_UTILS_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "statuses.hpp"

namespace Parse {

std::vector<std::string> Split(const std::string& text, char delimiter);

std::string ToLower(const std::string& text);

Status ParseSize(const std::string& text, const std::string& what, size_t& out);
Status ParseInt(const std::string& text, const std::string& what, int& out);

} // namespace Parse

#endif // PARSE_UTILS_HPP_
