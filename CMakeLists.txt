cmake_minimum_required (VERSION 3.20)

# vcpkg integration through env var
if(NOT DEFINED ENV{VCPKG_ROOT})
	message( FATAL_ERROR "Mesh requires VCPKG integration through VCPKG_ROOT env variable." )
endif()

message("VCPKG_ROOT is $ENV{VCPKG_ROOT}" )

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Generate compile_commands.json files so "Compile Active File" button provided by CMake extension works as expected
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

# project definition
project ("Mesh" 
	VERSION 1.0.1 
	DESCRIPTION "Mesh solutions"
	LANGUAGES CXX
)

# Enable testing
enable_testing()
set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST) # Postpone tests discovery to the moment they should be run

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


# Docker build target
# https://stackoverflow.com/questions/51474255/run-docker-build-in-cmake/55543728

#find_program(Docker_EXECUTABLE docker)

#if(NOT Docker_EXECUTABLE)
#    message(FATAL_ERROR "Cannot find the docker executable!")
#endif()

#add_custom_target(Storage_DockerImage
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${FFMPEG} ${CMAKE_CURRENT_LIST_DIR}
#    COMMAND ${CMAKE_COMMAND} -E copy ${FFMPEG_SO_INSTALL} ${FFMPEG}/lib

#	# Docker build command
#    COMMAND ${Docker_EXECUTABLE} build --tag Storage --file
#        ${CMAKE_CURRENT_LIST_DIR}/Dockerfile ${CMAKE_CURRENT_LIST_DIR}
#)

