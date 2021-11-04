#include "TSStorageClient.h"

#include <iostream>
#include <string>
#include <barrier>
#include <thread>
#include <iomanip>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#ifdef WIN32
#define timegm _mkgmtime
#endif

using namespace std;

extern const char *svcEndpoint;
std::shared_ptr<TSStorageClient> BuildClient();

namespace
{
    // Serie Id
    auto serieId = 1;

    // DateTime from
    time_t dtFrom = -1;

    // DateTime to
    time_t dtTo = -1;

    // Number of threads
    auto numberOfThreads = 1;

    // Synchronization point for threads
    std::unique_ptr<std::barrier<void (*)() noexcept>> syncPointPtr;

    // Number of all rows read
    std::atomic<int> atomicCount{0};

    // Protection for stdout operations
    std::mutex g_stdout_mutex;

    void ThreadProc()
    {
        // Prepare a client and open a connection
        auto client = BuildClient();

        //Signal thread is ready to send data
        syncPointPtr->arrive_and_wait();

        // Request data
        ::volue::mesh::protobuf::ReadPointsRequest request;

        // SerieId
        request.set_serie_id(serieId);

        // If an interval has been provided...
        if (dtFrom != -1)
        {
            // DateTime from
            {
                auto dt = new google::protobuf::Timestamp();
                dt->set_seconds(dtFrom);
                request.set_allocated_datetimefrom(dt);
            }

            // DateTime to
            {
                auto dt = new google::protobuf::Timestamp();
                dt->set_seconds(dtTo);
                request.set_allocated_datetimeto(dt);
            }
        }

        // Response data
        ::volue::mesh::protobuf::ReadPointsResponse response;

        // Read data
        auto rp_result = client->ReadPoints(request, &response);
        {
            std::lock_guard<std::mutex> lg(g_stdout_mutex);
            if (!rp_result.ok())
            {
                cout << "Thread error during ReadPoints(). The message is: " << rp_result.error_message() << endl;
                return;
            }

            // Report stats synchronized way
            cout << "Thread " << std::this_thread::get_id() << ", Avg speed is " << response.rows_per_sec() << endl;
            atomicCount += response.rows_read();
        }
    }
}

void ReadTest(int argc, const char **argv)
{
    cout << " - ReadPoints test" << endl;

    if (not(argc == 5 || argc == 7))
    {
        cout << endl
             << "Missing parameters!" << endl
             << endl;

        return;
    }

    // Service endpoint
    svcEndpoint = argv[2];

    // Number of threads
    numberOfThreads = std::stoi(argv[3]);
    auto syncFn = []() noexcept
    {
        std::cout << std::endl
                  << "All thread are ready to start the parallel reading operation" << std::endl
                  << std::endl;
    };
    syncPointPtr = std::make_unique<std::barrier<void (*)() noexcept>>(numberOfThreads, syncFn);

    // SerieId
    serieId = std::stoi(argv[4]);

    // DateTime from
    if (argc > 5)
    {
        std::stringstream ss{argv[5]};
        tm tm{};
        ss >> std::get_time(&tm, "%Y-%m-%d  %H:%M:%S");
        dtFrom = timegm(&tm);
    }

    // DateTime to
    if (argc > 6)
    {
        std::stringstream ss{argv[6]};
        tm tm{};
        ss >> std::get_time(&tm, "%Y-%m-%d  %H:%M:%S");
        dtTo = timegm(&tm);
    }

    // Construct client
    auto client = BuildClient();

    // Obtain version info
    volue::mesh::protobuf::MeshVersionInfo response;
    auto result = client->GetMeshVersion(&response);

    if (result.ok())
    {
        cout << "Product name is " << response.name() << ", the version is " << response.version() << endl;
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
    int rows_read = atomicCount.load();
    int64_t time_in_microsecs = duration_cast<std::chrono::microseconds>(execTime).count();
    double time_in_secs = time_in_microsecs / 1000'000.0;
    double rows_per_sec = rows_read / time_in_secs;

    cout << endl
         << "Client's perspective: " << endl;
    cout << "- number of rows read is " << rows_read << endl;
    cout << "- total time in seconds is " << time_in_secs << endl;
    cout << "- average speed is (in rows per sec) " << rows_per_sec << endl
         << endl;
}