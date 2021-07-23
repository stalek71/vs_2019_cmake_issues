# CMakeList.txt : CMake project for Mesh, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

# Protocol buffer
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Protocol buffer files
set(ProtoFiles 
	${ProtoDir}/Common/core_types.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${ProtoFiles})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS EXPORT_MACRO DLL_EXPORT ${ProtoFile})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS DESCRIPTORS PROTO_DESCS ${ProtoFile})

# Add source files to this project's executable.
add_executable (Storage 
	${PROTO_SRCS} 
	${PROTO_HDRS}
	Storage.cpp 
	Storage.h
)

# Libraries to link with
target_link_libraries(Storage
	PRIVATE Calculation ${Protobuf_LIBRARIES}
)

# Force the app to look in specific relative folder for needed shared libs (works on linux)
# To verify if it has been embedded in elf properly you can try this command: readelf -d Storage 
# To verify what folders have been searched for libs during runtime you can run this command: env LD_DEBUG=files,libs ./Storage 
# This command will show how dynamic deps have been resolved (so shorter form): ldd ./Storage 

#set_target_properties(Storage PROPERTIES
#    INSTALL_RPATH "../lib"
#    BUILD_WITH_INSTALL_RPATH 1
#)

install(TARGETS Storage)

# TODO: Add tests and install targets if needed.


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