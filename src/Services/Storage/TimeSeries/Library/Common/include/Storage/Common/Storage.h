#pragma once
#include <string>
#include <chrono>
#include <optional>

namespace mesh::storage::timeseries
{
    using Metadata = struct MetadataStruct
    {
        int serieId;
        uint8_t curveType;
        std::chrono::microseconds delta;

        int versionTS;
        uint8_t scopeType;
        std::optional<int> scopeId;
    };

    using timestamp = struct _ts
    {
        // Represents seconds of UTC time since Unix epoch
        // 1970-01-01T00:00:00Z. Must be from 0001-01-01T00:00:00Z to
        // 9999-12-31T23:59:59Z inclusive.
        int64_t seconds;

        // Non-negative fractions of a second at nanosecond resolution. Negative
        // second values with fractions must still have non-negative nanos values
        // that count forward in time. Must be from 0 to 999,999,999
        // inclusive.
        int32_t nanos;
    };

    using StorePointData = struct _pd
    {
        timestamp dateTime;
        double value;
        int privateFlags;
        int propagatingFlags;
    };
}
