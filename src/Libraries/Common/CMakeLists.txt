cmake_minimum_required (VERSION 3.20)

# Public interface files directory
set(pf "include/Common")

# Interface files (public files)
set(publicFiles 
	${pf}/Core/TimePoint.h
	${pf}/TimeSeries/TSPoint.h
	${pf}/TimeSeries/TimeSeries.h
)

# Private project files
set(privateFiles
	Core/TimePoint.cpp
	TimeSeries/TSPoint.cpp
	TimeSeries/TimeSeries.cpp
)

add_library(Common
	${publicFiles} 
	${privateFiles}
)

# Add bin dir to include paths
target_include_directories(Common PUBLIC 
	"${CMAKE_CURRENT_BINARY_DIR}" # Add a file with COMMON_EXPORT definition to the include search path
	include # Library public interface definitions
)

# Precompiled header definitions for public and private scopes
target_precompile_headers(Common 
	PUBLIC 
		${pf}/Core/TimePoint.h 
		<chrono>
	PRIVATE 
		<unordered_map>
)

# Write Common_export.h to the current binary directory (in case we would like to use this lib as a shared library)
include(GenerateExportHeader)
generate_export_header(Common)

install(TARGETS Common)