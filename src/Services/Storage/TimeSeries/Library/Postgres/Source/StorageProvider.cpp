#include <TimeSeriesStorage/StorageProvider.h>
#include <cstring>
#include <iostream>
#include <ctime>
#include <time.h>
#include <mutex>

#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>

// Temporary hacks for linux
#define htonll(x) ((1 == htonl(1)) ? (x) : ((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1 == ntohl(1)) ? (x) : ((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))

uint64_t htond(const double &value)
{
    uint64_t result;
    memcpy(&result, &value, sizeof(uint64_t));
    return htonll(result);
}

double ntohd(const uint64_t &value)
{
    double result;
    uint64_t tmp = ntohll(value);

    memcpy(&result, &tmp, sizeof(uint64_t));
    return result;
}
#endif

using namespace mesh::storage::timeseries;

namespace
{
    // Ids for prepared statements
    auto CREATE_SERIE_STMT = "create_serie";
    auto STORE_METADATA = "store_metadata";
    auto STORE_POINTS = "store_points";
    auto GET_NEXT_TS = "get_next_ts";

    auto getPoints_ALL_Sql = R"(
        COPY (
                SELECT  datetime,
                        value,
                        private_flags,
                        propagating_flags,
                        version_ts,
                        scope_type,
                        scope_id
                FROM ts.POINT
                WHERE serie_id = $PAR_SERIE_ID
            ) TO STDOUT (FORMAT binary);
      )";

    auto getPoints_INTERVAL_EXCLUDING_LAST_Sql = R"(
        COPY (
                SELECT  datetime,
                        value,
                        private_flags,
                        propagating_flags,
                        version_ts,
                        scope_type,
                        scope_id
                FROM ts.POINT 
                WHERE 
                    serie_id = $PAR_SERIE_ID
                    AND datetime BETWEEN '$PAR_DT_FROM' AND '$PAR_DT_TO'
            ) TO STDOUT (FORMAT binary);
      )";

    auto getPoints_INTERVAL_INCLUDING_LAST_Sql = R"(
        COPY (
                SELECT  datetime,
                        value,
                        private_flags,
                        propagating_flags,
                        version_ts,
                        scope_type,
                        scope_id
                FROM ts.POINT 
                WHERE 
                    serie_id = $PAR_SERIE_ID
                    AND 
                    (
                        datetime BETWEEN '$PAR_DT_FROM' AND '$PAR_DT_TO'
                        OR datetime = '$PAR_DT_TO'
                    )
            ) TO STDOUT (FORMAT binary);
      )";

    // Function to convert timestamp to postgres form used with binary bufers
    int64_t GetPGTime(const timestamp &timeVal)
    {
        // Timestamp on postgres in binary buffer form is a signed 64-bit integer representing microseconds from 2000-01-01 00:00:00.000000 (negative value means time before 2000)

        auto year2000_unix_seconds = 946684800; //January 1, 2000 UNIX time is 946684800

        // timeVal.seconds - int64_t, represents seconds of UTC time since Unix epoch
        // timeVal.nanos -  int32_t, fractions of a second at nanosecond resolution

        int64_t result = int64_t((timeVal.seconds - year2000_unix_seconds) * 1000'000 // Number of microseconds from Jan 1'st, 2000
                                 + (timeVal.nanos / 1000));                           //  nanoseconds should be "rounded" to microseconds

        return result;
    }

    timestamp GetTSTime(const int64_t &timeVal)
    {
        // Timestamp on postgres in binary buffer form is a signed 64-bit integer representing microseconds from 2000-01-01 00:00:00.000000 (negative value means time before 2000)

        auto year2000_unix_seconds = 946684800; //January 1, 2000 UNIX time is 946684800

        // timeVal.seconds - int64_t, represents seconds of UTC time since Unix epoch
        // timeVal.nanos -  int32_t, fractions of a second at nanosecond resolution

        timestamp result;
        result.seconds = (timeVal / 1000'000) + year2000_unix_seconds;
        result.nanos = (timeVal % 1000'000) * 1000; // Microseconds should be expanded to nanoseconds

        return result;
    }

    //  Protection against concurrent usage of gmtime
    std::mutex g_gmtime_mutex;

    // Conversion of timestamp to string
    std::string GetTimestampString(timestamp &timeVal)
    {
        // '2004-10-19 10:23:54.775'
        // timeVal.seconds - int64_t, represents seconds of UTC time since Unix epoch
        // timeVal.nanos -  int32_t, fractions of a second at nanosecond resolution

        auto tp = std::chrono::system_clock::from_time_t(timeVal.seconds);
        tp += std::chrono::microseconds(timeVal.nanos / 1000);

        //auto r1 = std::chrono::format("{:%Y-%m-%d %X}", tp);
        //auto const time = std::chrono::current_zone()->to_sys(tp);

        // Date/time value with precision to seconds
        std::time_t tt = timeVal.seconds;
        std::tm tm{};
        {
            std::lock_guard<std::mutex> lg(g_gmtime_mutex); // TODO Temporary hack for gmtime until thread safe version is available in std
            std::memcpy(&tm, std::gmtime(&tt), sizeof(tm));
        }

        char strBuff[30]{};
        std::strftime(&strBuff[0], sizeof(strBuff), "%Y-%m-%d %H:%M:%S", &tm);
        std::string result(strBuff);

        // Fraction of the second
        if (timeVal.nanos > 0)
        {
            double secFrac = timeVal.nanos / 1'000'000'000.0;
            std::string fracBuff = std::to_string(secFrac);
            result.append(fracBuff, 1, fracBuff.length() - 1);
        }

        return result;
    }

    // String replacement
    std::size_t ReplaceString(std::string &inout, std::string_view what, std::string_view with)
    {
        std::size_t count{};
        for (std::string::size_type pos{};
             inout.npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length(), ++count)
        {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
        return count;
    }
}

StorageProvider::StorageProvider(const std::string &connectionString) : connectionString_{connectionString}
{
    connPtr_ = PQconnectdb(connectionString.c_str());

    if (PQstatus(connPtr_) != CONNECTION_OK)
        throw std::runtime_error("Can't connect to db!");

    // Lambda for statement preparation
    auto prepare = [this](const char *name, const char *sql, int params = 0, Oid *paramTypes = nullptr)
    {
        auto result = PQprepare(connPtr_, name, sql, params, paramTypes);
        if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
            PQclear(result);
            throw std::runtime_error(PQerrorMessage(connPtr_));
        }

        PQclear(result);
    };

    prepare(CREATE_SERIE_STMT, "INSERT INTO ts.serie(name, point_type) VALUES($1::varchar, $2::\"char\") RETURNING id::int;");
    prepare(STORE_METADATA, "INSERT INTO ts.metadata(serie_id, curve_type, delta, version_ts, scope_type, scope_id) VALUES($1, $2, $3::interval, $4, $5, $6);");
    prepare(STORE_POINTS, "COPY ts.point(serie_id, datetime, value, private_flags, propagating_flags, version_ts, scope_type, scope_id) FROM STDIN (FORMAT binary);");
    prepare(GET_NEXT_TS, "SELECT NEXTVAL('data_version_ts');");
}

StorageProvider::~StorageProvider()
{
    if (connPtr_)
        PQfinish(connPtr_);
}

void StorageProvider::VerifyResult(PGresult *result, const ExecStatusType status) const
{
    if (PQresultStatus(result) != status)
    {
        PQclear(result);
        throw std::runtime_error(PQerrorMessage(connPtr_));
    }
}

void StorageProvider::VerifyClearResult(PGresult *result, const ExecStatusType status) const
{
    VerifyResult(result, status);
    PQclear(result);
}

void StorageProvider::BeginTx() const
{
    VerifyClearResult(PQexec(connPtr_, "BEGIN"));
}

void StorageProvider::CommitTx() const
{
    VerifyClearResult(PQexec(connPtr_, "END"));
}

void StorageProvider::RollbackTx() const
{
    VerifyClearResult(PQexec(connPtr_, "ROLLBACK"));
}

int StorageProvider::GetNextVersionTS()
{
    auto result = PQexecPrepared(connPtr_, GET_NEXT_TS, 0, nullptr, nullptr, nullptr, 1);
    if (PQresultStatus(result) == PGRES_TUPLES_OK) // PGRES_SINGLE_TUPLE
    {
        auto id = (int)ntohll(*(int64_t *)PQgetvalue(result, 0, 0));
        PQclear(result);

        return id;
    }

    // Something went wrong so throw exception
    PQclear(result);
    auto msg = PQerrorMessage(connPtr_);

    throw std::runtime_error(msg);
    return 0;
}

int StorageProvider::CreateSerie(const std::string &name, int8_t pointType) const
{
    BeginTx();

    const char *values[] = {name.c_str(), (const char *)&pointType};
    const int lengths[] = {(int)name.length() + 1, 1}; // size of binary values in bytes
    const int paramFormats[] = {0, 1};                 // 0 -text, 1 - binary

    auto result = PQexecPrepared(
        connPtr_,
        CREATE_SERIE_STMT,

        2,            //int nParams,
        values,       //const char * const *paramValues,
        lengths,      //const int *paramLengths,
        paramFormats, //const int *paramFormats,
        1);           //int resultFormat);

    if (PQresultStatus(result) == PGRES_TUPLES_OK) // PGRES_SINGLE_TUPLE
    {
        CommitTx();

        auto id = ntohl(*(long *)PQgetvalue(result, 0, 0));
        PQclear(result);

        return id;
    }

    // Something went wrong so throw exception
    PQclear(result);
    auto msg = PQerrorMessage(connPtr_);

    RollbackTx();
    throw std::runtime_error(msg);
}

// This is a definition of interval in binary form
// months field is a number of FULL months only. In a case when we have 30 months it means 2 years and 6 months
// time field contains number of microseconds for remaining part of interval (so for days, hours, minutes, seconds and fractional part of the second)
typedef struct
{
    int64_t time; /* all time units other than months and years */
    int days;
    int months; /* months and years, after time for alignment */
} pgdb_interval;

// This is timestamp definition used by postgres
typedef uint64_t pgdb_timestamp;

void StorageProvider::StoreMetadata(const Metadata &metadata) const
{
    BeginTx();

    // Delta value support - db uses INTERVAL type.
    pgdb_interval delta;

    // auto wholeMonths = std::chrono::duration_cast<std::chrono::months>(metadata.delta);
    // auto wholeDays = std::chrono::duration_cast<std::chrono::days>(metadata.delta - wholeMonths);
    // auto remainingMicroSecs = std::chrono::duration_cast<std::chrono::microseconds>(metadata.delta - wholeMonths - wholeDays);

    // auto orig = wholeMonths + wholeDays + remainingMicroSecs; // for comparison with original only

    auto deltaInMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(metadata.delta).count();
    auto remainingTimeInMics = deltaInMicroseconds;

    auto microsecsInMonth = 30ll * 24 * 60 * 60 * 1000'000;
    auto wholeMonths = remainingTimeInMics / microsecsInMonth;
    remainingTimeInMics = remainingTimeInMics % microsecsInMonth;

    auto microsecsInDay = 24ll * 60 * 60 * 1000'000;
    auto wholeDays = remainingTimeInMics / microsecsInDay;
    remainingTimeInMics = remainingTimeInMics % microsecsInDay;

    // Conversion from std::chrono::duration to interval
    delta.months = htonl(wholeMonths);
    delta.days = htonl(wholeDays);
    delta.time = htonll(remainingTimeInMics);

    int seriesId = htonl(metadata.serieId);
    int versionTS = htonl(metadata.versionTS);
    int scopeId = metadata.scopeId.has_value() ? htonl(metadata.scopeId.value()) : 0;

    // Param values (positional)
    const char *values[] =
        {
            (const char *)&seriesId,
            (const char *)&metadata.curveType, // 1 byte
            (const char *)&delta,
            (const char *)&versionTS,
            (const char *)&metadata.scopeType,                                // 1 byte
            (const char *)(metadata.scopeId.has_value() ? &scopeId : nullptr) // Can be NULL
        };

    // Param sizes in bytes
    const int lengths[] =
        {
            sizeof(metadata.serieId),
            1,
            sizeof(delta),
            sizeof(metadata.versionTS),
            1,
            sizeof(int)};

    // Param formats
    const int paramFormats[] = {1, 1, 1, 1, 1, 1}; // 0 - text, 1 - binary

    auto result = PQexecPrepared(
        connPtr_,
        STORE_METADATA,
        6,            //int nParams,
        values,       //const char * const *paramValues,
        lengths,      //const int *paramLengths,
        paramFormats, //const int *paramFormats,
        1);           //int resultFormat);

    // Commit changes on success
    if (PQresultStatus(result) == PGRES_COMMAND_OK)
    {
        PQclear(result);
        CommitTx();
    }
    else
    {
        // Something went wrong so throw exception
        PQclear(result);
        auto msg = PQerrorMessage(connPtr_);

        RollbackTx();
        throw std::runtime_error(msg);
    }
}

std::tuple<int, int64_t, double> StorageProvider::StorePoints(int serieId, int versionTS, uint8_t scopeType, std::optional<int> scopeId, int len, const StorePointData *points) const
{
    BeginTx();

    auto result = PQexecPrepared(connPtr_, STORE_POINTS, 0, nullptr, nullptr, nullptr, 1);
    VerifyClearResult(result, PGRES_COPY_IN);

    char header[12] = "PGCOPY\n\377\r\n\0";
    int flag = 0;
    int extension = 0;

    constexpr int short_size = sizeof(short);
    constexpr int int_size = sizeof(int);
    constexpr int int64_size = sizeof(int64_t);
    constexpr int dbl_size = sizeof(double);

    auto pointSize = sizeof(short)  // number of cols in tuple (8 cols currently)
                     + 8 * int_size // col length
                     + int_size     // int serie_id
                     + int64_size   // int64_t dateTime
                     + dbl_size     // double value
                     + int_size     // int private_flags
                     + int_size     // int propagating_flags
                     + int_size     // int version_ts
                     + 1            // uint8_t scope_type
                     + (scopeId.has_value()
                            ? int_size
                            : 0); // int scope_id

    // Total buffer size
    auto buffSize = 11 + 2 * int_size + len * pointSize + short_size; // Header + flags + points + file trailer

    // Buffer allocation - will be automatically deallocated
    std::unique_ptr<char[]> buffPtr(new char[buffSize]);

    // Pointer to use during the buffer traversal
    char *tmpPtr = buffPtr.get();

    // Header
    memcpy(tmpPtr, header, 11);
    tmpPtr += 11;

    // Flag
    memcpy(tmpPtr, (void *)&flag, 4);
    tmpPtr += int_size;

    // Extension
    memcpy(tmpPtr, (void *)&extension, 4);
    tmpPtr += int_size;

    // Lambda to set uint8_t value in the buffer
    auto setByteVal = [&](uint8_t val)
    {
        uint8_t *intPtr = (uint8_t *)tmpPtr;
        *intPtr = val;
        tmpPtr += sizeof(uint8_t);
    };

    // Lambda to set short value in the buffer
    auto setShortVal = [&](short val)
    {
        short *intPtr = (short *)tmpPtr;
        *intPtr = htons(val);
        tmpPtr += short_size;
    };

    // Lambda to set int32_t value in the buffer
    auto setIntVal = [&](int val)
    {
        int *intPtr = (int *)tmpPtr;
        *intPtr = htonl(val);
        tmpPtr += int_size;
    };

    // Lambda to set int64_t value in the buffer
    auto setInt64Val = [&](const int64_t val)
    {
        int64_t *intPtr = (int64_t *)tmpPtr;
        *intPtr = htonll(val);
        tmpPtr += int64_size;
    };

    // Lambda to set double value in the buffer
    auto setDoubleVal = [&](const double val)
    {
        uint64_t *ptr = (uint64_t *)tmpPtr;
        *ptr = htond(val);
        tmpPtr += dbl_size;
    };

    // For every point
    for (int pos = 0; pos < len; ++pos)
    {
        auto &point = points[pos];

        // Number of cols in row (short)
        setShortVal(8);

        // serie_id column (int)
        setIntVal(int_size); // Col size
        setIntVal(serieId);  // Col val

        // dateTime column (int64_t)
        setIntVal(int64_size); // Col size
        setInt64Val(GetPGTime(point.dateTime));

        // value column (double)
        setIntVal(dbl_size);
        setDoubleVal(point.value);

        // private_flags column (int)
        setIntVal(int_size);           // Col size
        setIntVal(point.privateFlags); // Col val

        // propagating_flags column (int)
        setIntVal(int_size);               // Col size
        setIntVal(point.propagatingFlags); // Col val

        // version_ts column (int)
        setIntVal(int_size);  // Col size
        setIntVal(versionTS); // Col val

        // scope_type column (uint8_t)
        setIntVal(1);          // Col size
        setByteVal(scopeType); // Col val

        // scope_id column (int - can be NULL)
        if (scopeId.has_value())
        {
            setIntVal(int_size);        // Col size
            setIntVal(scopeId.value()); // Col val
        }
        else
        {
            // Col size: -1 means NULL value
            setIntVal(-1);
        }
    }

    // File trailer
    setShortVal(-1);

    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    // Start measuring execution time
    auto t1 = high_resolution_clock::now();

    // Send data to server
    auto copyRes = PQputCopyData(connPtr_, buffPtr.get(), buffSize);
    if (copyRes == 1)
    {
        if (PQputCopyEnd(connPtr_, NULL) == 1)
        {
            auto res = PQgetResult(connPtr_);
            if (PQresultStatus(res) == PGRES_COMMAND_OK)
            {
                PQclear(res);
                CommitTx();
            }
            else
            {
                PQclear(res);
                auto msg = PQerrorMessage(connPtr_);

                RollbackTx();
                throw std::runtime_error(msg);
            }
        }
        else
        {
            auto msg = PQerrorMessage(connPtr_);
            RollbackTx();
            throw std::runtime_error(msg);
        }
    }
    else if (copyRes == 0)
    {
        RollbackTx();
        throw std::runtime_error("Send no data, connection is in nonblocking mode");
    }
    else if (copyRes == -1)
    {
        auto msg = PQerrorMessage(connPtr_);
        RollbackTx();
        throw std::runtime_error(msg);
    }

    // Stop measuring execution time
    auto t2 = high_resolution_clock::now();
    auto execTime = t2 - t1;

    // Stats to send back
    int rows_inserted = len;
    int64_t time_in_microsecs = duration_cast<std::chrono::microseconds>(execTime).count();
    double time_in_secs = time_in_microsecs / 1000'000.0;
    double rows_per_sec = rows_inserted / time_in_secs;

    // Stats as a tupple to unfold on a usage
    return std::make_tuple(rows_inserted, time_in_microsecs, rows_per_sec);
}

std::tuple<int, int64_t, double> StorageProvider::ReadPoints(int serieId,
                                                             std::optional<timestamp> &dateTimeFrom, std::optional<timestamp> &dateTimeTo, std::optional<bool> &includingEndPoint,
                                                             std::function<void(timestamp, double, int, int, int, uint8_t, std::optional<int> &)> storeRowInGrpcBuf)
{
    // Statement to compose
    std::string sql;

    // Detect case
    if (dateTimeFrom.has_value()) // Do we have dateTime values provided at all?
    {
        //dateTimeFromParam = htonll(GetPGTime(dateTimeFrom.value()));
        //dateTimeToParam = htonll(GetPGTime(dateTimeTo.value()));

        // Should we include the last one point also?
        if (includingEndPoint.has_value() && includingEndPoint.value() == true)
            sql = getPoints_INTERVAL_INCLUDING_LAST_Sql; // Include last point
        else
            sql = getPoints_INTERVAL_EXCLUDING_LAST_Sql; // Exclude last point

        // Params replacement
        ReplaceString(sql, "$PAR_SERIE_ID", std::to_string(serieId));
        ReplaceString(sql, "$PAR_DT_FROM", GetTimestampString(dateTimeFrom.value()));
        ReplaceString(sql, "$PAR_DT_TO", GetTimestampString(dateTimeTo.value()));
    }
    else
    {
        // No dateTime values provided
        sql = getPoints_ALL_Sql; //

        const char serieIdParName[] = "$PAR_SERIE_ID";
        sql.replace(sql.find(serieIdParName), sizeof(serieIdParName), std::to_string(serieId));
    }

    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    // Start measuring execution time
    auto t1 = high_resolution_clock::now();

    // Execute prepared statement
    auto result = PQexecParams(connPtr_, sql.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1); // Output format - binary

    // Ensure it's in COPY_OUT state
    VerifyClearResult(result, PGRES_COPY_OUT);

    // Count rows for stats
    auto rowsCount = 0;

    // Buffer pointer
    char *buffer;

    // Tmp pointer
    char *ptr;

    // Lambda to read value size. -1 means null value
    auto getValSize = [&]()
    {
        int *intPtr = (int *)ptr;
        auto result = ntohl(*intPtr);
        ptr += sizeof(int);
        return result;
    };

    auto firstRowOffset = 11 + 2 * sizeof(int) + sizeof(short);
    auto nextRowOffset = sizeof(short);

    // One row only per a call
    for (;;)
    {
        auto getBuffResult = PQgetCopyData(connPtr_, &buffer, 0); // 0 means it's sync version

        // Skip empty result set or File Trailer
        if (getBuffResult == firstRowOffset || getBuffResult == 2 && *(short *)buffer == -1)
        {
            PQfreemem(buffer);
            continue;
        }

        // Do we have any data? -1 means that operation has been completed
        if (getBuffResult == -1)
        {
            result = PQgetResult(connPtr_);
            break;
        }

        // Error?
        if (getBuffResult == -2)
        {
            PQclear(result);

            auto msg = PQerrorMessage(connPtr_);
            throw std::runtime_error(msg);
        }

        // Setup a pointer for the first one column data
        ptr = buffer + ((rowsCount == 0) ? firstRowOffset : nextRowOffset); // Skip buffer header and col infos

        // Decode values
        ptr += sizeof(int); // Skip value size as it's always provided
        timestamp dateTime = GetTSTime(*(int64_t *)ptr);
        ptr += sizeof(int64_t);

        //ptr += sizeof(int); // Skip value size as it's always provided
        auto valueSize = getValSize();
        double value = ntohd(*(uint64_t *)ptr);
        ptr += sizeof(int64_t);

        ptr += sizeof(int); // Skip value size as it's always provided
        int privateFlags = ntohl(*(uint32_t *)ptr);
        ptr += sizeof(uint32_t);

        ptr += sizeof(int); // Skip value size as it's always provided
        int propagatingFlags = ntohl(*(uint32_t *)ptr);
        ptr += sizeof(uint32_t);

        ptr += sizeof(int); // Skip value size as it's always provided
        int versionTS = ntohl(*(uint32_t *)ptr);
        ptr += sizeof(uint32_t);

        ptr += sizeof(int); // Skip value size as it's always provided
        uint8_t scopeType = *(uint8_t *)ptr;
        ptr += sizeof(uint8_t);

        // Construct scope_id value
        std::optional<int> scopeId;
        auto scopeIdSize = getValSize();
        if (scopeIdSize != -1)
        {
            int scopeIdValue = ntohl(*(uint32_t *)ptr);
            ptr += sizeof(uint32_t);
            scopeId = scopeIdValue;
        }

        // Store decoded value directly in GRPC output buffer
        storeRowInGrpcBuf(
            dateTime,
            value,
            privateFlags,
            propagatingFlags,
            versionTS,
            scopeType,
            scopeId);

        // Release the buffer allocated for row data
        if (getBuffResult > 0)
        {
            PQfreemem(buffer);
            ++rowsCount;
        }
    }

    // Verify final result
    VerifyClearResult(result, PGRES_COMMAND_OK);

    // Stop measuring execution time
    auto t2 = high_resolution_clock::now();
    auto execTime = t2 - t1;

    // Stats to send back
    int rows_read = rowsCount;
    int64_t time_in_microsecs = duration_cast<std::chrono::microseconds>(execTime).count();
    double time_in_secs = time_in_microsecs / 1000'000.0;
    double rows_per_sec = rows_read / time_in_secs;

    // Stats as a tupple to unfold on a usage
    return std::make_tuple(rows_read, time_in_microsecs, rows_per_sec);
}