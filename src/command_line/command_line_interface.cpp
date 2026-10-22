#include "command_line_interface.hpp"

#include <iostream>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

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
};

struct OptionSpec {
    std::string_view name;
    Option option;
    bool needs_value;
};

constexpr OptionSpec kOptions[] = {
    {"--single", Option::kSingle, true},
    {"--multi",  Option::kMulti,  true},
    {"--costs",  Option::kCosts,  true},
    {"--length", Option::kLength, true},
    {"--seeds",  Option::kSeeds,  true},
    {"--out",    Option::kOut,    true},
    {"-h",       Option::kHelp,   false},
    {"--help",   Option::kHelp,   false},
};

const OptionSpec* FindOption(std::string_view text) {
    for (const OptionSpec& spec : kOptions) {
        if (text == spec.name) {
            return &spec;
        }
    }

    return nullptr;
}

Status ParsePositiveInt(std::string_view text, std::string_view what, int& out) {
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

Status ParseSetupWithArity(std::string_view text, CacheFactory::Setup& setup, size_t min_levels,
                           size_t max_levels, std::string_view flag, std::string_view expectation) {
    const Status status = CacheFactory::ParseSetup(text, setup);
    if (status != Status::kOk) {
        return status;
    }

    if (setup.size() < min_levels || setup.size() > max_levels) {
        std::cerr << "error: " << flag << " expects " << expectation
                  << ", got " << setup.size() << "\n";
        return Status::kWrongLevelCount;
    }

    return Status::kOk;
}

Status ParseCosts(std::string_view text, std::vector<size_t>& costs) {
    std::vector<size_t> parsed;
    for (std::string_view part : Parse::Split(text, ',')) {
        size_t cost = 0;
        const Status status = Parse::ParseSize(part, "cost", cost);
        if (status != Status::kOk) {
            return status;
        }

        parsed.push_back(cost);
    }

    costs = std::move(parsed);
    return Status::kOk;
}

Status ApplyOption(Option option, std::string_view value, Config& config) {
    switch (option) {
        case Option::kSingle:
        case Option::kMulti: {
            CacheFactory::Setup setup;

            const Status status = (option == Option::kSingle)
                ? ParseSetupWithArity(value, setup, 1, 1, "--single",
                                      "exactly one cache (use --multi for a hierarchy)")
                : ParseSetupWithArity(value, setup, 2, std::numeric_limits<size_t>::max(),
                                      "--multi",
                                      "at least two levels (use --single for one cache)");

            if (status != Status::kOk) {
                return status;
            }

            config.setups.push_back(std::move(setup));
            return Status::kOk;
        }

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
            return Status::kOk;
    }

    return Status::kOk;
}

std::vector<CacheFactory::Setup> DefaultSetups() {
    constexpr size_t kL1Size = 3;
    constexpr size_t kL2Size = 15;

    const auto types = CacheFactory::AllCacheTypes();

    std::vector<CacheFactory::Setup> setups;
    setups.reserve(types.size() * types.size());

    for (CacheFactory::CacheType first : types) {
        for (CacheFactory::CacheType second : types) {
            CacheFactory::Setup setup;
            setup.reserve(2);
            setup.push_back({first, kL1Size});
            setup.push_back({second, kL2Size});
            setups.push_back(std::move(setup));
        }
    }

    return setups;
}

Status ValidateCosts(const Config& config) {
    if (config.costs.empty()) {
        return Status::kOk;
    }

    if (config.setups.size() > 1) {
        std::cerr << "error: --costs applies to a single configuration, but "
                  << config.setups.size() << " were given"
                     " (pass exactly one --single or --multi)\n";
        return Status::kCostsMismatch;
    }

    const CacheFactory::Setup& setup = config.setups.front();
    if (config.costs.size() != setup.size() + 1) {
        std::cerr << "error: --costs needs " << setup.size() + 1 << " values for '"
                  << CacheFactory::Describe(setup) << "' (" << setup.size()
                  << " levels + miss cost), got " << config.costs.size() << "\n";
        return Status::kCostsMismatch;
    }

    return Status::kOk;
}

} // namespace

Status ParseArgs(int argc, char** argv, Config& config) {
    CLI::ArgCursor args(argc, argv);

    while (args.HasNext()) {
        const std::string_view text = args.Next();
        const OptionSpec* spec = FindOption(text);

        if (spec == nullptr) {
            std::cerr << "error: unknown option '" << text << "'\n";
            return Status::kUnknownOption;
        }

        if (spec->option == Option::kHelp) {
            config.show_help = true;
            return Status::kOk;
        }

        std::string_view value;
        if (spec->needs_value) {
            const Status status = args.NextValue(text, value);
            if (status != Status::kOk) {
                return status;
            }
        }

        const Status status = ApplyOption(spec->option, value, config);
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
