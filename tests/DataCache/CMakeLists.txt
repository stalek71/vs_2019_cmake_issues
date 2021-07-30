# DataCache library tests
cmake_minimum_required (VERSION 3.20)

# Find gtest
find_package(GTest REQUIRED)

# Add current output dir to include dirs
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Private files directory
set(pf "Source")

# Private project files
set(privateFiles
	${pf}/main.cpp
    ${pf}/DataCacheParametrizedTests.cpp
    ${pf}/DataCacheMockSample.cpp
)

# Add source files to this project's executable.
add_executable (CacheSvcTests
	${privateFiles}
)

# Libraries to link with
target_link_libraries(CacheSvcTests
	PRIVATE 
		Common
		DataCache
        #${GTEST_LIBRARIES}
        GTest::gtest
        GTest::gmock
        #GTest::gtest_main
)

gtest_discover_tests(
    CacheSvcTests
    #EXTRA_ARGS --algo=fast
    #TEST_SUFFIX .Fast
    #TEST_LIST fastTests
    PROPERTIES "LABELS;core;LABELS;cache"
)