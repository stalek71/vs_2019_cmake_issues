#include "gtest/gtest.h"
#include <TimeSeriesStorage/StorageProvider.h>
#include <chrono>

#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace mesh::storage::timeseries;
using namespace std::chrono_literals;

TEST(TSSTorage, CreateSerie)
{
    StorageProvider ps("postgresql://postgres:StAlek678213@localhost/mesh");
    auto serieId = ps.CreateSerie("Serie_1", 1);

    EXPECT_NE(serieId, 0);
}

TEST(TSSTorage, StoreMetadata)
{
    StorageProvider ps("postgresql://postgres:StAlek678213@localhost/mesh");
    Metadata md{
        .serieId = 15,
        .curveType = 1,
        .delta = 123 * 24h + 8h + 30min + 30s,
        .versionTS = 30,
        .scopeType = 50,
        .scopeId = 250};

    ps.StoreMetadata(md);
}

TEST(TSSTorage, StorePoints)
{
    StorageProvider ps("postgresql://postgres:StAlek678213@localhost/mesh");

    auto year2000_unix_seconds = 946684800;
    auto ts_2000_01_01_14_30_35 = year2000_unix_seconds + (14 * 60 * 60) + (30 * 60) + 35;
    auto ts_2000_01_01_20_00_05 = year2000_unix_seconds + (20 * 60 * 60) + 5;

    timestamp year_1{ts_2000_01_01_14_30_35, 0};
    timestamp year_2{ts_2000_01_01_20_00_05, 0};

    StorePointData points[] = {
        {year_1,
         123.45,
         77,
         88},
        {year_2,
         123.45,
         77,
         88}};

    ps.StorePoints(15, 30, 50, 100, sizeof(points) / sizeof(StorePointData), points);
}

TEST(TSSTorage, GetNextTS)
{
    StorageProvider ps("postgresql://postgres:StAlek678213@localhost/mesh");
    auto nextTS = ps.GetNextVersionTS();

    EXPECT_NE(nextTS, 0);
}

constexpr int year2000_unix_seconds = 946684800;

TEST(TSSTorage, PerfTest)
{
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    StorageProvider ps("postgresql://postgres:StAlek678213@localhost/mesh");

    constexpr int numberOfPoints = 1'000'000;

    std::unique_ptr<StorePointData[]> pointsPtr(new StorePointData[numberOfPoints]);
    auto points = pointsPtr.get();

    for (int pos = 0; pos < numberOfPoints; ++pos)
    {
        auto point = points + pos;

        point->dateTime = timestamp(year2000_unix_seconds + pos * 5);
        point->privateFlags = 100 + pos;
        point->propagatingFlags = 200 + pos;
        point->value = 1.0 * pos;
    }

    auto t1 = high_resolution_clock::now();
    ps.StorePoints(5, 51, 50, 100, numberOfPoints, points);
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;

    auto execTimeInSecs = ms_double.count() / 1000;
    auto rowsPerSec = numberOfPoints / execTimeInSecs;

    int k = 0;
}

TEST(TSSTorage, ReadPoints)
{
    StorageProvider ps("postgresql://postgres:StAlek678213@localhost/mesh");

    std::optional<timestamp> dtFrom = timestamp(year2000_unix_seconds + 360);
    std::optional<timestamp> dtTo = timestamp(year2000_unix_seconds + 720);
    std::optional<bool> includeEndpoint = true;

    auto assignData = [](timestamp ts, double value, int flag_1, int flag_2, int versionTS, uint8_t scope_type, std::optional<int> &scope_id)
    {
        int k = 0;
        int j = k;
    };

    // 11.5 sec dla single row
    // 6.6 sec dla COPY TO STDOUT (BINARY)

    auto result = ps.ReadPoints(5, dtFrom, dtTo, includeEndpoint, assignData);
    int w = 0;
}
