#include "parse_utils.hpp"
#include "statuses.hpp"

#include <cctype>
#include <charconv>
#include <iostream>
#include <system_error>

namespace Parse {

namespace {

    template <typename T>
    Status ParseNumber(std::string_view text, std::string_view what, T& out) {
        T value = 0;
        const char* const begin = text.data();
        const char* const end = begin + text.size();

        const std::from_chars_result result = std::from_chars(begin, end, value);

        if (result.ec == std::errc::result_out_of_range) {
            std::cerr << "error: " << what << " is out of range: '" << text << "'\n";
            return Status::kInvalidNumber;
        }

        if (result.ec != std::errc() || result.ptr != end) {
            std::cerr << "error: invalid " << what << ": '" << text << "'\n";
            return Status::kInvalidNumber;
        }

        out = value;
        return Status::kOk;
    }

} // namespace

std::vector<std::string_view> Split(std::string_view text, char delimiter) {
    std::vector<std::string_view> parts;
    size_t begin = 0;

    while (true) {
        const size_t pos = text.find(delimiter, begin);
        if (pos == std::string_view::npos) {
            parts.push_back(text.substr(begin));
            break;
        }

        parts.push_back(text.substr(begin, pos - begin));
        begin = pos + 1;
    }

    return parts;
}

std::string ToLower(std::string_view text) {
    std::string result(text);
    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    return result;
}

Status ParseSize(std::string_view text, std::string_view what, size_t& out) {
    return ParseNumber(text, what, out);
}

Status ParseInt(std::string_view text, std::string_view what, int& out) {
    return ParseNumber(text, what, out);
}

} // namespace Parse
