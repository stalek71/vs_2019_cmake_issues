#pragma once

#include "timeseries_storage.grpc.pb.h"

namespace mesh::service::TSStorage
{
    class TimeSeriesStorageService final : public volue::mesh::protobuf::TimeSeriesStorage::Service
    {
    public:
        TimeSeriesStorageService();

        virtual ::grpc::Status GetMeshVersion(::grpc::ServerContext *context, const ::google::protobuf::Empty *request, ::volue::mesh::protobuf::MeshVersionInfo *response) override;

        virtual ::grpc::Status CreateSerie(::grpc::ServerContext* context, const ::volue::mesh::protobuf::CreateSerieRequest* request, ::volue::mesh::protobuf::CreateSerieResponse* response) override;
        virtual ::grpc::Status StoreMetadata(::grpc::ServerContext* context, const ::volue::mesh::protobuf::StoreMetadataRequest* request, ::google::protobuf::Empty* response) override;
        virtual ::grpc::Status StorePoints(::grpc::ServerContext* context, const ::volue::mesh::protobuf::StorePointsRequest* request, ::google::protobuf::Empty* response) override;
    };
}