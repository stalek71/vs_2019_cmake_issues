#include "StorageSvc.h"
#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using namespace mesh::service::TSStorage;

TimeSeriesStorageService::TimeSeriesStorageService()
{
}

grpc::Status TimeSeriesStorageService::GetMeshVersion(::grpc::ServerContext *context,const ::google::protobuf::Empty *request,::volue::mesh::protobuf::MeshVersionInfo *response)
{
	response->set_name("Mesh-TimeSeriesStorage-Service");
	response->set_version("1.0.extreme");

	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::CreateSerie(::grpc::ServerContext* context, const ::volue::mesh::protobuf::CreateSerieRequest* request, ::volue::mesh::protobuf::CreateSerieResponse* response) 
{
	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::StoreMetadata(::grpc::ServerContext* context, const ::volue::mesh::protobuf::StoreMetadataRequest* request, ::google::protobuf::Empty* response)
{
	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::StorePoints(::grpc::ServerContext* context, const ::volue::mesh::protobuf::StorePointsRequest* request, ::google::protobuf::Empty* response)
{
	return Status::OK;
}