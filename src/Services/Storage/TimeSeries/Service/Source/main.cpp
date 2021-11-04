#include <TimeSeriesStorage/StorageProvider.h>
#include <iostream>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "StorageSvc.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

void RunServer() 
{
  std::string server_address("0.0.0.0:8080");
  mesh::service::TSStorage::TimeSeriesStorageService service;

  ServerBuilder builder;
  builder.SetMaxReceiveMessageSize(INT_MAX);
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main()
{
	cout << "Hello from TimeSeries storage service!" << endl;
    RunServer();
	return 0;
}