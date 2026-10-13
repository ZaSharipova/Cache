#ifndef CLI_HPP_
#define CLI_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "CacheFactory.hpp"
#include "statuses.hpp"

namespace CLI {

struct Config {
    std::vector<CacheFactory::Setup> setups;
    std::vector<size_t> costs;
    int trace_length = 10000;
    int seeds = 30;
    std::string output_path = "csvs/results.csv";
    bool show_help = false;
};

const char* PrintUsageText();
Status ParseArgs(int argc, char** argv, Config& config);

class ArgCursor {
public:
    ArgCursor(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            args_.push_back(argv[i]);
        }
    }

    bool HasNext() const {
        return index_ < args_.size();
    }

    const std::string& Next() {
        return args_[index_++];
    }

    Status NextValue(const std::string& option, std::string& value) {
        if (!HasNext()) {
            std::cerr << "error: missing value for " << option << "\n";
            return Status::kMissingValue;
        }

        value = Next();
        return Status::kOk;
    }

private:
    std::vector<std::string> args_;
    size_t index_ = 0;
};

} // namespace CLI

#endif // CLI_HPP_
