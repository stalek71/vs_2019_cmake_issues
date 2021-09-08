#pragma once

#include "data_cache.grpc.pb.h"

namespace mesh::service::DataCache
{
    class DataCacheService final : public volue::mesh::protobuf::DataCache::Service
    {
    public:
        DataCacheService();

        virtual ::grpc::Status GetMeshVersion(
            ::grpc::ServerContext* context, 
            const ::google::protobuf::Empty* request, 
            ::volue::mesh::protobuf::MeshVersionInfo* response) override;
    };
}