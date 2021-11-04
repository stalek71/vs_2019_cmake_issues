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

extern const char *svcEndpoint;
std::shared_ptr<TSStorageClient> BuildClient();

namespace
{
    // Serie Id
    auto serieId = 1;

    // Number of points to store per thread
    auto pointsToStore = 100'000;

    // Number of threads
    auto numberOfThreads = 1;

    // Synchronization point for threads
    std::unique_ptr<std::barrier<void (*)() noexcept>> syncPointPtr;

    // Protection for stdout operations
    std::mutex g_stdout_mutex;

    void ThreadProc()
    {
        // Prepare a client and open a connection
        auto client = BuildClient();

        // Get next version number for data versioning
        ::volue::mesh::protobuf::GetNextDataSetTSResponse nextTSResponse;
        auto resp_result = client->GetNextDataSetTS(&nextTSResponse);
        if (not resp_result.ok())
        {
            {
                std::lock_guard<std::mutex> lg(g_stdout_mutex);
                cout << "Thread error during GetNextDataSetTS(). The message is: " << resp_result.error_message() << endl;
            }
            return;
        }

        // NextTS value from db managed sequence
        int nextTS = nextTSResponse.next_ts();

        {
            std::lock_guard<std::mutex> lg(g_stdout_mutex);
            cout << "Thread " << std::this_thread::get_id() << ", Data VersionID is " << nextTS << endl;
        }

        //Signal thread is ready to send data
        syncPointPtr->arrive_and_wait();

        // Request data
        ::volue::mesh::protobuf::StorePointsRequest request;
        request.set_serie_id(serieId);
        request.set_version_ts(nextTS);
        request.set_scope_type(::volue::mesh::protobuf::ScopeType::GLOBAL);
        request.set_scope_id(230);

        auto year2000_unix_seconds = 946684800;

        for (int pos = 0; pos < pointsToStore; ++pos)
        {
            auto point = request.add_points();
            auto dt = new google::protobuf::Timestamp();
            dt->set_seconds(year2000_unix_seconds + pos * 5);
            point->set_allocated_datetime(dt);
            point->set_private_flags(100 + pos);
            point->set_propagating_flags(200 + pos);
            point->set_value(1.0 * pos);
        }

        // Response data
        ::volue::mesh::protobuf::StorePointsResponse response;

        // Send data
        auto sp_result = client->StorePoints(request, &response);
        {
            std::lock_guard<std::mutex> lg(g_stdout_mutex);
            if (!sp_result.ok())
            {
                cout << "Thread error during StorePoints(). The message is: " << resp_result.error_message() << endl;
                return;
            }

            // Report stats synchronized way
            cout << "Thread " << std::this_thread::get_id() << ", Avg speed is " << response.rows_per_sec() << endl;
        }
    }
}

void WriteTest(int argc, const char **argv)
{
    cout << " - Write timeseries points test" << endl;

    // Service endpoint
    if (argc > 2)
        svcEndpoint = argv[2];

    // Number of points to store per thread
    if (argc > 3)
        pointsToStore = std::stoi(argv[3]);

    // Number of threads
    if (argc > 4)
        numberOfThreads = std::stoi(argv[4]);

    auto syncFn = []() noexcept
    {
        std::cout << std::endl
                  << "All thread are ready to start the parallel writing operation" << std::endl
                  << std::endl;
    };
    syncPointPtr = std::make_unique<std::barrier<void (*)() noexcept>>(numberOfThreads, syncFn);

    // SerieId
    if (argc > 5)
        serieId = std::stoi(argv[5]);

    // Construct client
    auto client = BuildClient();

    // Obtain version info
    volue::mesh::protobuf::MeshVersionInfo response;
    auto result = client->GetMeshVersion(&response);

    if (result.ok())
    {
        cout << "Product name is " << response.name() << ", the version is " << response.version() << endl
             << endl
             << "Data set versions for threads are:" << endl;
    }
    else
    {
        cout << "TimeSeriesStorage service is not responding. Are you sure it has been started at all?" << endl;
        cout << "Error message is: " << result.error_message() << endl;
    }

    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    // Start measuring execution time
    auto t1 = high_resolution_clock::now();

    // Construct and start all the threads
    std::vector<std::thread> threads;
    for (int pos = 0; pos < numberOfThreads; ++pos)
    {
        threads.emplace_back(ThreadProc);
    }

    // Wait for threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }

    // Stop measuring execution time
    auto t2 = high_resolution_clock::now();
    auto execTime = t2 - t1;

    // Stats to send back
    int rows_inserted = pointsToStore * numberOfThreads;
    int64_t time_in_microsecs = duration_cast<std::chrono::microseconds>(execTime).count();
    double time_in_secs = time_in_microsecs / 1000'000.0;
    double rows_per_sec = rows_inserted / time_in_secs;

    cout << endl
         << "Client's perspective: " << endl;
    cout << "- number of rows sent is " << rows_inserted << endl;
    cout << "- total time in seconds is " << time_in_secs << endl;
    cout << "- average speed is (in rows per sec) " << rows_per_sec << endl
         << endl;
}