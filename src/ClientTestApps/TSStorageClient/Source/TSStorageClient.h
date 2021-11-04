#pragma once

#include <memory>
#include <grpcpp/channel.h>
#include <timeseries_storage.grpc.pb.h>

class TSStorageClient
{
public:
    // Constructor
    TSStorageClient(std::shared_ptr<grpc::Channel> channel);

    // Obtain mesh version
    ::grpc::Status GetMeshVersion(::volue::mesh::protobuf::MeshVersionInfo *response);

    // Get next timestamp for data versioning
    ::grpc::Status GetNextDataSetTS(::volue::mesh::protobuf::GetNextDataSetTSResponse *response);

    // Send data points
    ::grpc::Status StorePoints(const ::volue::mesh::protobuf::StorePointsRequest &request, ::volue::mesh::protobuf::StorePointsResponse *response);

    // Read data points
    ::grpc::Status ReadPoints(const ::volue::mesh::protobuf::ReadPointsRequest &request, ::volue::mesh::protobuf::ReadPointsResponse *response);

private:
    std::unique_ptr<volue::mesh::protobuf::TimeSeriesStorage::Stub> stub_;
};