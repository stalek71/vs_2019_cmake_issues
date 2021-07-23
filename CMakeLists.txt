cmake_minimum_required (VERSION 3.20)

# vcpkg integration through env var
if(NOT DEFINED ENV{VCPKG_ROOT})
	message( FATAL_ERROR "Mesh requires VCPKG integration through VCPKG_ROOT env variable." )
endif()

message("VCPKG_ROOT is $ENV{VCPKG_ROOT}" )

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# project definition
project ("Mesh" 
	VERSION 1.0.1 
	DESCRIPTION "Mesh solutions"
	LANGUAGES CXX
)

# CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Default compiler settings (can be overriden in subfolders)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sample - preferred way for targets in reality
# target_compile_features(myTarget 
#	PUBLIC cxx_std_11
#)

include(GNUInstallDirs)

# Obtain relative path for libs dir
file(RELATIVE_PATH relDir
	${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
	${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
)

# Protocol files directory
set(ProtoDir ${CMAKE_SOURCE_DIR}/src/proto)

# Set search path for runtime deps to current (./) and ./lib folders
set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${relDir})

# Include sub-projects.
add_subdirectory ("src")
add_subdirectory ("tests")

# default startup project for Visual Studio
if (MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Storage)
endif()

