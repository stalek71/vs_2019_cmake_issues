#pragma once
#include <string>
#include <chrono>

namespace mesh::storage::timeseries
{
    using Metadata = struct
    {
        int serieId;
        int8_t curveType;
        std::chrono::milliseconds delta;

        int versionTS;
        int8_t scopeType;
        int scopeId;
    };

    using DateTime_ms = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

    using PointData = struct
    {
        DateTime_ms dateTime;
        double value;
        int privateFlags;
        int propagatingFlags;
    };

    int CreateSerie(const std::string &name, int8_t pointType);
    void StoreMetadata(const Metadata &metadata);
    void StorePoints(int serieId, int versionTS, int len, const PointData* points);
}
