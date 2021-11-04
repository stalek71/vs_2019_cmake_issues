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
        virtual ::grpc::Status StorePoints(::grpc::ServerContext* context, const ::volue::mesh::protobuf::StorePointsRequest* request, ::volue::mesh::protobuf::StorePointsResponse* response) override;
        virtual ::grpc::Status GetNextDataSetTS(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, ::volue::mesh::protobuf::GetNextDataSetTSResponse* response) override;
        virtual ::grpc::Status ReadPoints(::grpc::ServerContext* context, const ::volue::mesh::protobuf::ReadPointsRequest* request, ::volue::mesh::protobuf::ReadPointsResponse* response) override;
    };
}