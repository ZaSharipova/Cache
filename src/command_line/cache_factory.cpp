#include "cache_factory.hpp"

#include <iostream>
#include <optional>

#include "2Qcache.hpp"
#include "arc_cache.hpp"
#include "lfu_cache.hpp"
#include "lirs_cache.hpp"
#include "lru_cache.hpp"
#include "parse_utils.hpp"

namespace CacheFactory {

namespace {

struct TypeName {
    CacheType type;
    const char* name;
};

const TypeName kTypeNames[] = {
    {CacheType::kLru, "lru"},
    {CacheType::kLfu, "lfu"},
    {CacheType::kTwoQ, "2q"},
    {CacheType::kLirs, "lirs"},
    {CacheType::kArc, "arc"},
};

std::optional<CacheType> FindCacheType(const std::string& name) {
    for (const TypeName& entry : kTypeNames) {
        if (name == entry.name) {
            return entry.type;
        }
    }

    return std::nullopt;
}

std::unique_ptr<Cache> MakeCache(const CacheSpec& spec) {
    switch (spec.type) {
        case CacheType::kLru:
            return std::make_unique<LRUCache<int, int>>(spec.size);
        case CacheType::kLfu:
            return std::make_unique<LFUCache<int, int>>(spec.size);
        case CacheType::kTwoQ:
            return std::make_unique<TwoQCache<int, int>>(spec.size);
        case CacheType::kLirs:
            return std::make_unique<LIRSCache<int, int>>(spec.size);
        case CacheType::kArc:
            return std::make_unique<ARCCache<int, int>>(spec.size);
    }

    return nullptr;
}

Status ParseCacheSpec(const std::string& text, CacheSpec& spec) {
    const size_t colon = text.find(':');
    if (colon == std::string::npos) {
        std::cerr << "error: expected <type>:<size>, got '" << text << "'\n";
        return Status::kInvalidSpec;
    }

    CacheSpec parsed;
    const Status status = Parse::ParseSize(text.substr(colon + 1), "cache size", parsed.size);
    if (status != Status::kOk) {
        return status;
    }

    if (parsed.size == 0) {
        std::cerr << "error: cache size must be positive in '" << text << "'\n";
        return Status::kInvalidCacheSize;
    }

    const std::string type_name = Parse::ToLower(text.substr(0, colon));
    if (auto type = FindCacheType(type_name); !type) {
        std::cerr << "error: unknown cache type '" << type_name << "' (see --help)\n";
        return Status::kUnknownCacheType;
    } else {
        parsed.type = *type;
    }

    spec = parsed;
    return Status::kOk;
}

std::vector<size_t> DefaultCosts(size_t level_count) {
    std::vector<size_t> costs;
    size_t cost = 1;
    for (size_t i = 0; i <= level_count; i++) {
        costs.push_back(cost);
        cost *= 10;
    }

    return costs;
}

} // namespace

std::vector<CacheType> AllCacheTypes() {
    std::vector<CacheType> types;
    for (const TypeName& entry : kTypeNames) {
        types.push_back(entry.type);
    }

    return types;
}

std::string CacheTypeName(CacheType type) {
    for (const TypeName& entry : kTypeNames) {
        if (type == entry.type) {
            return entry.name;
        }
    }

    return "";
}

Status ParseSetup(const std::string& text, Setup& setup) {
    Setup parsed;
    for (const std::string& part : Parse::Split(text, ',')) {
        CacheSpec spec;
        const Status status = ParseCacheSpec(part, spec);
        if (status != Status::kOk) {
            return status;
        }

        parsed.push_back(spec);
    }

    setup = parsed;
    return Status::kOk;
}

std::string Describe(const Setup& setup) {
    std::string description;
    for (const CacheSpec& spec : setup) {
        if (!description.empty()) {
            description += " -> ";
        }

        description += CacheTypeName(spec.type) + ":" + std::to_string(spec.size);
    }

    return description;
}

size_t TotalSize(const Setup& setup) {
    size_t total = 0;
    for (const CacheSpec& spec : setup) {
        total += spec.size;
    }
    return total;
}

Status MakeMultiLevel(const Setup& setup, std::vector<size_t> costs, std::unique_ptr<MLCache>& out) {
    if (costs.empty()) {
        costs = DefaultCosts(setup.size());
    }
    if (costs.size() != setup.size() + 1) {
        std::cerr << "error: '" << Describe(setup) << "' needs " << setup.size() + 1
                  << " costs (levels + miss), got " << costs.size() << "\n";
        return Status::kCostsMismatch;
    }

    std::vector<std::unique_ptr<Cache>> levels;
    for (const CacheSpec& spec : setup) {
        levels.push_back(MakeCache(spec));
    }

    out = std::make_unique<MLCache>(std::move(levels), std::move(costs));
    return Status::kOk;
}

} // namespace CacheFactory
