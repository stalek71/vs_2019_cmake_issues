# Cache library
cmake_minimum_required (VERSION 3.20)

# Public interface files directory
set(if "include/DataCache")

# Interface files (public files)
set(publicFiles 
	${if}/DataCache.h
)

# Private files directory
set(pf "Source")

# Private project files
set(privateFiles
	${pf}/DataCache.cpp
)

# Library definition
add_library(DataCache
	${publicFiles} 
	${privateFiles}
)

# Add bin dir to include paths
target_include_directories(DataCache PUBLIC 
	"${CMAKE_CURRENT_BINARY_DIR}" # Add a file with COMMON_EXPORT definition to the include search path
	include # Library public interface definitions
)

# Precompiled header definitions for public and private scopes
target_precompile_headers(DataCache 
	PUBLIC 
		${if}/DataCache.h 
		<chrono>
	PRIVATE 
		<unordered_map>
)

# Write Cache_export.h to the current binary directory (in case we would like to use this lib as a shared library)
include(GenerateExportHeader)
generate_export_header(DataCache)

install(TARGETS DataCache)