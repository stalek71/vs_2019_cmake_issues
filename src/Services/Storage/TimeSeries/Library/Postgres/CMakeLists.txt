# Storage library
cmake_minimum_required (VERSION 3.20)

find_package(OpenSSL REQUIRED)
find_package(PostgreSQL REQUIRED) # duplicated by libpqxx
#find_package(libpqxx REQUIRED)

# Public interface files directory
set(if "include/TimeSeriesStorage")

# Interface files (public files)
set(publicFiles 
	${if}/StorageProvider.h
)

# Private files directory
set(pf "Source")

# Private project files
set(privateFiles
	${pf}/StorageProvider.cpp
)

# Library definition
add_library(TimeSeriesStorageOnPostgres
	${publicFiles} 
	${privateFiles}
)

target_link_libraries(TimeSeriesStorageOnPostgres 
	PUBLIC StorageCommon
	#PUBLIC libpqxx::pqxx
	PUBLIC PostgreSQL::PostgreSQL
	PUBLIC OpenSSL::SSL 
	PUBLIC OpenSSL::Crypto
)

if(WIN32)
  target_link_libraries(TimeSeriesStorageOnPostgres
  	PUBLIC wsock32 
	PUBLIC ws2_32)
endif()


# Add bin dir to include paths
target_include_directories(TimeSeriesStorageOnPostgres PUBLIC 
	"${CMAKE_CURRENT_BINARY_DIR}" # Add a file with COMMON_EXPORT definition to the include search path
	include # Library public interface definitions
)

# Precompiled header definitions for public and private scopes
target_precompile_headers(TimeSeriesStorageOnPostgres 
	PUBLIC 
		${if}/StorageProvider.h 
		<chrono>
	PRIVATE 
		<unordered_map>
)

# Write Storage_export.h to the current binary directory (in case we would like to use this lib as a shared library)
include(GenerateExportHeader)
generate_export_header(TimeSeriesStorageOnPostgres)

install(TARGETS TimeSeriesStorageOnPostgres)