#include "CacheSvc.h"
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

using namespace mesh::service::DataCache;

DataCacheService::DataCacheService()
{

}
grpc::Status DataCacheService::GetMeshVersion(
	::grpc::ServerContext *context,
	const ::google::protobuf::Empty *request,
	::volue::mesh::protobuf::MeshVersionInfo *response)
{
	// TODO: Move whole implementation to DataCache library and integrate with it
	response->set_name("Mesh-DataCacheService");
	response->set_version("1.0.extreme");

	return Status::OK;
}