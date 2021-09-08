# Storage library
cmake_minimum_required (VERSION 3.20)

find_package(PostgreSQL REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

# Public interface files directory
set(if "include/TimeSeriesStorage")

# Interface files (public files)
set(publicFiles 
	${if}/Storage.h
)

# Private files directory
set(pf "Source")

# Private project files
set(privateFiles
	${pf}/Storage.cpp
)

# Library definition
add_library(TimeSeriesStorage
	${publicFiles} 
	${privateFiles}
)

target_link_libraries(TimeSeriesStorage PUBLIC PostgreSQL::PostgreSQL)
target_link_libraries(TimeSeriesStorage PUBLIC libpqxx::pqxx)

# Add bin dir to include paths
target_include_directories(TimeSeriesStorage PUBLIC 
	"${CMAKE_CURRENT_BINARY_DIR}" # Add a file with COMMON_EXPORT definition to the include search path
	include # Library public interface definitions
)

# Precompiled header definitions for public and private scopes
target_precompile_headers(TimeSeriesStorage 
	PUBLIC 
		${if}/Storage.h 
		<chrono>
	PRIVATE 
		<unordered_map>
)

# Write Storage_export.h to the current binary directory (in case we would like to use this lib as a shared library)
include(GenerateExportHeader)
generate_export_header(TimeSeriesStorage)

install(TARGETS TimeSeriesStorage)