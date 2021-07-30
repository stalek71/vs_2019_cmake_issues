#include "gtest/gtest.h"
#include <DataCache/DataCache.h>

using namespace mesh::DataCache;

// In case you need complex params - just use tupples
class DataCacheParameterizedTestFixture : public ::testing::TestWithParam<int>
{
protected:
    DataCache cache_;
};

TEST_P(DataCacheParameterizedTestFixture, GetVersion)
{
    int compatibilityParam = GetParam();
    auto result = cache_.GetVersion(compatibilityParam);

    ASSERT_TRUE(result != "NoVersion");
}

INSTANTIATE_TEST_CASE_P(
    DataCacheParameterizedTests,
    DataCacheParameterizedTestFixture,
    ::testing::Values(
        1, 2, 5, 10));