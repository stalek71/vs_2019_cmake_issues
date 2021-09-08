#include "CacheSvcClient.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using namespace volue::mesh::protobuf;

CacheSvcClient::CacheSvcClient(std::shared_ptr<grpc::Channel> channel)
 : stub_(volue::mesh::protobuf::DataCache::NewStub(channel))
{
}

grpc::Status CacheSvcClient::GetMeshVersion(volue::mesh::protobuf::MeshVersionInfo* response)
{
    grpc::ClientContext context;
    return stub_->GetMeshVersion(&context, google::protobuf::Empty(), response);
}