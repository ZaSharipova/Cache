#ifndef STATUS_HPP_
#define STATUS_HPP_

enum class Status {
    kOk = 0,
    kIoError,
    kInvalidNumber,
    kNonPositiveValue,
    kInvalidSpec,
    kInvalidCacheSize,
    kUnknownCacheType,
    kWrongLevelCount,
    kCostsMismatch,
    kUnknownOption,
    kMissingValue,
};

#endif // STATUS_HPP_
