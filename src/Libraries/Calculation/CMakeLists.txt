cmake_minimum_required (VERSION 3.20)

# This is library project that should be created as a shared library, because Calculation service is linking with it generated calculation code. 
# When we have shared library (*.dll, *.so etc) to link with it should reduce expression build time.

# Public interface files directory
set(pf "include/Calc/")

# Interface files (public files)
set(publicFiles 
	${pf}Calculation.h
)

# Private project files
set(privateFiles
	Calculation.cpp
)

add_library(Calculation SHARED 
	${publicFiles} 
	${privateFiles}
)

# Add bin dir to include paths
target_include_directories(Calculation PUBLIC 
	"${CMAKE_CURRENT_BINARY_DIR}" # Add a file with CALCULATION_EXPORT definition to the include search path
	include # Library public interface definitions
)

# Precompiled header definitions for public and private scopes
target_precompile_headers(Calculation 
	PUBLIC ${pf}Calculation.h
	PRIVATE <unordered_map>
)

# Write Calculation_export.h to the current binary directory
include(GenerateExportHeader)
generate_export_header(Calculation)

install(TARGETS Calculation)