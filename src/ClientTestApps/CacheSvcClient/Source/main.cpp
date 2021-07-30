#include "CacheSvcClient.h"

#include <iostream>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using namespace std;

int main()
{
    CacheSvcClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    volue::mesh::protobuf::MeshVersionInfo response;
    auto result = client.GetMeshVersion(&response);

    if (result.ok())
    {
        cout << "Product name is " << response.name() << ", version is " << response.version() << endl;
    }
    else
    {
        cout << "DataCache service is not responding. Are you sure it has been started at all?" << endl;
        cout << "Error message is: " << result.error_message() << endl;
    }

    return 0;
}