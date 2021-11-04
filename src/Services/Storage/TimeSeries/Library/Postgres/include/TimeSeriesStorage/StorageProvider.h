#pragma once
#include <Storage/Common/Storage.h>
#include <libpq-fe.h>
#include <memory>
#include <optional>
#include <tuple>
#include <functional>

namespace mesh::storage::timeseries
{
    class StorageProvider
    {
    public:
        StorageProvider(const std::string &connectionString);
        ~StorageProvider();

        // Get next data version id
        int GetNextVersionTS();

        // Store operations
        int CreateSerie(const std::string &name, int8_t pointType) const;
        void StoreMetadata(const Metadata &metadata) const;
        // Tuple elems: 0 = rows inserted (int), 1 = time in microsecs (int64), 2 = rows per sec (double)
        std::tuple<int, int64_t, double> StorePoints(int serieId, int versionTS, uint8_t scopeType, std::optional<int> scopeId, int len, const StorePointData *points) const;

        // Read operations (the result in tuple like in StorePoints above)
        std::tuple<int, int64_t, double> ReadPoints(
            int serieId,
            std::optional<timestamp> &dateTimeFrom, std::optional<timestamp> &dateTimeTo, std::optional<bool> &includingEndPoint,
            std::function<void(timestamp, double, int, int, int, uint8_t, std::optional<int> &)> storeRowInGrpcBuf);

    private:
        void BeginTx() const;
        void RollbackTx() const;
        void CommitTx() const;
        void VerifyResult(PGresult *result, const ExecStatusType status = PGRES_COMMAND_OK) const;
        void VerifyClearResult(PGresult *result, const ExecStatusType status = PGRES_COMMAND_OK) const;

        std::string connectionString_;
        PGconn *connPtr_; // to be able to prepare/compile and use prepared statements
    };
}