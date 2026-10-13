#include "parse_utils.hpp"
#include "statuses.hpp"

#include <cctype>
#include <iostream>
#include <limits>

namespace Parse {

namespace {

Status FailParse(const std::string& what, const std::string& text) {
    std::cerr << "error: invalid " << what << ": '" << text << "'\n";
    return Status::kInvalidNumber;
}

template <typename T>
Status ParseUnsigned(const std::string& text, const std::string& what, T& out) {
    if (text.empty()) {
        return FailParse(what, text);
    }

    T value = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return FailParse(what, text);
        }

        const T digit = static_cast<T>(ch - '0');
        if (value > (std::numeric_limits<T>::max() - digit) / 10) {
            return FailParse(what, text);
        }

        value = value * 10 + digit;
    }

    out = value;
    return Status::kOk;
}

} // namespace

std::vector<std::string> Split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    size_t begin = 0;

    while (true) {
        const size_t pos = text.find(delimiter, begin);
        if (pos == std::string::npos) {
            parts.push_back(text.substr(begin));
            break;
        }

        parts.push_back(text.substr(begin, pos - begin));
        begin = pos + 1;
    }

    return parts;
}

std::string ToLower(const std::string& text) {
    std::string result(text);
    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(ch));
    }

    return result;
}

Status ParseSize(const std::string& text, const std::string& what, size_t& out) {
    return ParseUnsigned(text, what, out);
}

Status ParseInt(const std::string& text, const std::string& what, int& out) {
    return ParseUnsigned(text, what, out);
}

} // namespace Parse
