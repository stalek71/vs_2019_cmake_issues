# StorageSvc
cmake_minimum_required (VERSION 3.20)

# Protocol buffer
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Protocol buffer files
set(protoFiles 
	${ProtoDir}/Common/core_types.proto
	${ProtoDir}/Storage/timeseries_storage.proto
)

# Private files directory
set(pf "Source")

# Private project files
set(privateFiles
	${pf}/StorageSvc.h
	${pf}/StorageSvc.cpp
	${pf}/main.cpp
)

# Add source files to this project's executable.
add_executable (TimeSeriesStorageSvc 
	${privateFiles}
	${protoFiles}
)

# Libraries to link with
target_link_libraries(TimeSeriesStorageSvc
	PRIVATE 
		Common
		TimeSeriesStorageOnPostgres
		protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)


# protobuf_generate function definition is available under the link below:
# https://github.com/protocolbuffers/protobuf/blob/master/cmake/protobuf-config.cmake.in
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(TARGET TimeSeriesStorageSvc APPEND_PATH LANGUAGE cpp)
protobuf_generate(TARGET TimeSeriesStorageSvc APPEND_PATH LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")


# Force the app to look in specific relative folder for needed shared libs (works on linux)
# To verify if it has been embedded in elf properly you can try this command: readelf -d TimeSeriesStorageSvc 
# To verify what folders have been searched for libs during runtime you can run this command: env LD_DEBUG=files,libs ./TimeSeriesStorageSvc 
# This command will show how dynamic deps have been resolved (so shorter form): ldd ./TimeSeriesStorageSvc 

#set_target_properties(TimeSeriesStorageSvc PROPERTIES
#    INSTALL_RPATH "../lib"
#    BUILD_WITH_INSTALL_RPATH 1
#)

install(TARGETS TimeSeriesStorageSvc)
