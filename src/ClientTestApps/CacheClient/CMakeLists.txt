# CacheSvc
cmake_minimum_required (VERSION 3.20)

# Protocol buffer
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Protocol buffer files
set(protoFiles 
	${ProtoDir}/Common/core_types.proto
	${ProtoDir}/Cache/data_cache.proto
)

# Private files directory
set(pf "Source")

# Private project files
set(privateFiles
	${pf}/CacheSvcClient.h
	${pf}/CacheSvcClient.cpp
	${pf}/main.cpp
)

# Add source files to this project's executable.
add_executable (CacheClient
	${privateFiles}
	${protoFiles}
)

# Libraries to link with
target_link_libraries(CacheClient
	PRIVATE 
		protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)

# protobuf_generate function definition is available under the link below:
# https://github.com/protocolbuffers/protobuf/blob/master/cmake/protobuf-config.cmake.in
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(TARGET CacheClient APPEND_PATH LANGUAGE cpp)
protobuf_generate(TARGET CacheClient APPEND_PATH LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Installation
install(TARGETS CacheClient)
