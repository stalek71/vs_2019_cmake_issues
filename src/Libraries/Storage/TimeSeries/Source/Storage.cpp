#include <TimeSeriesStorage/Storage.h>
#include <pqxx/pqxx>

namespace mesh::storage::timeseries
{
    int CreateSerie(const std::string &name, int8_t pointType)
    {
        pqxx::connection conn("postgresql://postgres:StAlek678213@localhost/mesh");
        pqxx::work work{conn};

        auto row = work.exec1("SELECT count(*) FROM ts.serie");
        auto resp = row[0].c_str();

        return 0;
    }

    void StoreMetadata(const Metadata &metadata)
    {
    }

    void StorePoints(int serieId, int versionTS, int len, const PointData *points)
    {
    }
}