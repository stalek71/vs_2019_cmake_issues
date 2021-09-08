#include "gtest/gtest.h"
#include <TimeSeriesStorage/Storage.h>

using namespace mesh::storage::timeseries;

TEST(TSSTorage, CreateSerie)
{
    auto serieId = CreateSerie("Serie_1", 1);
    EXPECT_EQ(0, 0);
}