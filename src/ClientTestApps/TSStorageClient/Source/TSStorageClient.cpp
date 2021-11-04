#include "TSStorageClient.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using namespace volue::mesh::protobuf;

TSStorageClient::TSStorageClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(volue::mesh::protobuf::TimeSeriesStorage::NewStub(channel))
{
}

grpc::Status TSStorageClient::GetMeshVersion(volue::mesh::protobuf::MeshVersionInfo *response)
{
    grpc::ClientContext context;
    return stub_->GetMeshVersion(&context, google::protobuf::Empty(), response);
}

::grpc::Status TSStorageClient::StorePoints(const ::volue::mesh::protobuf::StorePointsRequest &request, ::volue::mesh::protobuf::StorePointsResponse *response)
{
    grpc::ClientContext context;
    return stub_->StorePoints(&context, request, response);
}

::grpc::Status TSStorageClient::GetNextDataSetTS(::volue::mesh::protobuf::GetNextDataSetTSResponse *response)
{
    grpc::ClientContext context;
    return stub_->GetNextDataSetTS(&context, ::google::protobuf::Empty(), response);
}

::grpc::Status TSStorageClient::ReadPoints(const ::volue::mesh::protobuf::ReadPointsRequest &request, ::volue::mesh::protobuf::ReadPointsResponse *response)
{
    grpc::ClientContext context;
    return stub_->ReadPoints(&context, request, response);
}