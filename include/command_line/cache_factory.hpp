#ifndef CacheFactory_HPP_
#define CacheFactory_HPP_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "ICache.hpp"
#include "multilevel_cache.hpp"
#include "statuses.hpp"

namespace CacheFactory {

using Cache = ICache<int, int>;
using MLCache = MultiLevelCache<int, int>;

enum class CacheType {
    kLru,
    kLfu,
    kTwoQ,
    kLirs,
    kArc,
};

struct CacheSpec {
    CacheType type = CacheType::kLru;
    size_t size = 0;
};

using Setup = std::vector<CacheSpec>;

std::vector<CacheType> AllCacheTypes();
std::string CacheTypeName(CacheType type);
Status ParseSetup(const std::string& text, Setup& setup);
std::string Describe(const Setup& setup);
size_t TotalSize(const Setup& setup);
Status MakeMultiLevel(const Setup& setup, std::vector<size_t> costs, std::unique_ptr<MLCache>& out);

} // namespace CacheFactory

#endif // CacheFactory_HPP_
