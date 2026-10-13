#include "command_line_interface.hpp"

#include <iostream>

#include "parse_utils.hpp"

namespace CLI {
namespace {

enum class Option {
    kSingle,
    kMulti,
    kCosts,
    kLength,
    kSeeds,
    kOut,
    kHelp,
    kUnknown,
};

Option ParseOption(const std::string& text) {
    if (text == "--single") {
        return Option::kSingle;
    }

    if (text == "--multi") {
        return Option::kMulti;
    }

    if (text == "--costs") {
        return Option::kCosts;
    }

    if (text == "--length") {
        return Option::kLength;
    }

    if (text == "--seeds") {
        return Option::kSeeds;
    }

    if (text == "--out") {
        return Option::kOut;
    }

    if (text == "-h" || text == "--help") {
        return Option::kHelp;
    }

    return Option::kUnknown;
}

Status ParsePositiveInt(const std::string& text, const std::string& what, int& out) {
    int value = 0;
    const Status status = Parse::ParseInt(text, what, value);
    if (status != Status::kOk) {
        return status;
    }

    if (value <= 0) {
        std::cerr << "error: " << what << " must be positive, got '" << text << "'\n";
        return Status::kNonPositiveValue;
    }

    out = value;
    return Status::kOk;
}

Status ParseSingle(const std::string& text, CacheFactory::Setup& setup) {
    const Status status = CacheFactory::ParseSetup(text, setup);
    if (status != Status::kOk) {
        return status;
    }

    if (setup.size() != 1) {
        std::cerr << "error: --single expects exactly one cache, got " << setup.size()
                  << " (use --multi for a hierarchy)\n";
        return Status::kWrongLevelCount;
    }

    return Status::kOk;
}

Status ParseMulti(const std::string& text, CacheFactory::Setup& setup) {
    const Status status = CacheFactory::ParseSetup(text, setup);
    if (status != Status::kOk) {
        return status;
    }

    if (setup.size() < 2) {
        std::cerr << "error: --multi expects at least two levels (use --single for one cache)\n";
        return Status::kWrongLevelCount;
    }

    return Status::kOk;
}

Status ParseCosts(const std::string& text, std::vector<size_t>& costs) {
    std::vector<size_t> parsed;
    for (const std::string& part : Parse::Split(text, ',')) {
        size_t cost = 0;
        const Status status = Parse::ParseSize(part, "cost", cost);
        if (status != Status::kOk) {
            return status;
        }

        parsed.push_back(cost);
    }

    costs = parsed;
    return Status::kOk;
}

Status ApplyOption(Option option, const std::string& value, Config& config) {
    CacheFactory::Setup setup;
    Status status = Status::kOk;

    switch (option) {
        case Option::kSingle:
            status = ParseSingle(value, setup);
            if (status == Status::kOk) {
                config.setups.push_back(setup);
            }
            return status;

        case Option::kMulti:
            status = ParseMulti(value, setup);
            if (status == Status::kOk) {
                config.setups.push_back(setup);
            }
            return status;

        case Option::kCosts:
            return ParseCosts(value, config.costs);

        case Option::kLength:
            return ParsePositiveInt(value, "trace length", config.trace_length);

        case Option::kSeeds:
            return ParsePositiveInt(value, "seeds count", config.seeds);

        case Option::kOut:
            config.output_path = value;
            return Status::kOk;

        case Option::kHelp:
        case Option::kUnknown:
            break;
    }

    return Status::kUnknownOption;
}

std::vector<CacheFactory::Setup> DefaultSetups() {
    const size_t kL1Size = 3;
    const size_t kL2Size = 15;

    std::vector<CacheFactory::Setup> setups;
    for (CacheFactory::CacheType first : CacheFactory::AllCacheTypes()) {
        for (CacheFactory::CacheType second : CacheFactory::AllCacheTypes()) {
            CacheFactory::Setup setup;
            setup.push_back({first, kL1Size});
            setup.push_back({second, kL2Size});
            setups.push_back(setup);
        }
    }

    return setups;
}

Status ValidateCosts(const Config& config) {
    if (config.costs.empty()) {
        return Status::kOk;
    }

    for (const CacheFactory::Setup& setup : config.setups) {
        if (config.costs.size() != setup.size() + 1) {
            std::cerr << "error: --costs needs " << setup.size() + 1 << " values for '"
                      << CacheFactory::Describe(setup) << "' (" << setup.size()
                      << " levels + miss cost), got " << config.costs.size() << "\n";

            return Status::kCostsMismatch;
        }
    }

    return Status::kOk;
}

} // namespace

Status ParseArgs(int argc, char** argv, Config& config) {
    CLI::ArgCursor args(argc, argv);

    while (args.HasNext()) {
        const std::string text = args.Next();
        const Option option = ParseOption(text);

        if (option == Option::kHelp) {
            config.show_help = true;
            return Status::kOk;
        }

        if (option == Option::kUnknown) {
            std::cerr << "error: unknown option '" << text << "'\n";
            return Status::kUnknownOption;
        }

        std::string value;
        Status status = args.NextValue(text, value);
        if (status != Status::kOk) {
            return status;
        }

        status = ApplyOption(option, value, config);
        if (status != Status::kOk) {
            return status;
        }
    }

    if (config.setups.empty()) {
        config.setups = DefaultSetups();
    }

    return ValidateCosts(config);
}

} // namesapce CLI
