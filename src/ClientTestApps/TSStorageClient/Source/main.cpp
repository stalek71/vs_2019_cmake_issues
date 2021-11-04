#include "TSStorageClient.h"

#include <iostream>
#include <string>
#include <barrier>
#include <thread>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using namespace std;

// Service endpoint
const char *svcEndpoint = "localhost:8080";

// GRPC client factory
std::shared_ptr<TSStorageClient> BuildClient()
{
    // Prepare a client and open a connection
    grpc::ChannelArguments channelArgs;
    channelArgs.SetMaxReceiveMessageSize(-1); // No limit on mesage size
    auto channel = grpc::CreateCustomChannel(svcEndpoint, grpc::InsecureChannelCredentials(), channelArgs);
    auto connResult = channel.get()->GetState(true); // True to connect if not connected yet...

    return std::make_shared<TSStorageClient>(channel);
}

void ReadTest(int argc, const char **argv);
void WriteTest(int argc, const char **argv);

int main(int argc, const char **argv)
{
    cout << endl
         << "TimeSeries Storage Service test ";

    // {
    //     const char *args[]{
    //         "nazwa",
    //         "read",
    //         "localhost:8080",
    //         "5",
    //         "2000-01-01 00:00:20",
    //         "2000-01-03 00:03:00",
    //         "1"};

    //     ReadTest(7, args);
    // }

    // Test type
    if (argc == 1)
    {
        cout << endl << "Missing parameter - test type (can be \"read\" or \"write\")" << endl
             << endl;
        return -1;
    }

    auto testTypeCode = argv[1][0];

    if (testTypeCode == 'r' || testTypeCode == 'R')
        ReadTest(argc, argv);

    if (testTypeCode == 'w' || testTypeCode == 'W')
        WriteTest(argc, argv);

    return 0;
}