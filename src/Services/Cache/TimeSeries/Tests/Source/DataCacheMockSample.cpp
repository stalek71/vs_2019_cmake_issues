#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <DataCache/IDataCache.h>

// Open the testing namespace
using namespace ::testing;

// Mock definition
// Some interesting info is here:
// http://google.github.io/googletest/gmock_cook_book.html#NiceStrictNaggy
class DataCacheMock : public mesh::DataCache::IDataCache
{
public:
    MOCK_METHOD(std::string, GetVersion, (int compatibilityFlag), (override));
};

// Tests
TEST(DataCacheMockTest, CanDoSomething)
{
    // Declare mock instance
    DataCacheMock dcMock;

    // Declare expectation on mock
    EXPECT_CALL(dcMock, GetVersion(_)).Times(AtLeast(1));

    // Use mock
    auto result = dcMock.GetVersion(20);

    //EXPECT_TRUE(something);
}