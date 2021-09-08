#pragma once

#include <memory>
#include <grpcpp/channel.h>
#include <data_cache.grpc.pb.h>

class CacheSvcClient
{
public:
    // Constructor
    CacheSvcClient(std::shared_ptr<grpc::Channel> channel);

    // Obtain mesh version
    ::grpc::Status GetMeshVersion(::volue::mesh::protobuf::MeshVersionInfo* response);

private:
    std::unique_ptr<volue::mesh::protobuf::DataCache::Stub> stub_;
};