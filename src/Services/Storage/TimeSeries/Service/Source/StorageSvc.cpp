#include "StorageSvc.h"
#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <Storage/Common/ProviderFactory.hpp>

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using namespace mesh::service::TSStorage;
using namespace mesh::storage::timeseries;

static ProviderFactory<StorageProvider, 20> providerFactory( // Max 20 paralled db connections
	[]()
	{
		auto envConnStr = std::getenv("DB_CONN_STRING");
		return std::make_unique<StorageProvider>(envConnStr ? envConnStr : "postgresql://postgres:StAlek678213@localhost/mesh");
	});

TimeSeriesStorageService::TimeSeriesStorageService()
{
}

grpc::Status TimeSeriesStorageService::GetMeshVersion(::grpc::ServerContext *, const ::google::protobuf::Empty *, ::volue::mesh::protobuf::MeshVersionInfo *response)
{
	response->set_name("Mesh-TimeSeriesStorage-Service");
	response->set_version("1.0.extreme");

	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::CreateSerie(::grpc::ServerContext *context, const ::volue::mesh::protobuf::CreateSerieRequest *request, ::volue::mesh::protobuf::CreateSerieResponse *response)
{
	try
	{
		auto provider = providerFactory.Get();
		auto result = provider->CreateSerie(request->name(), request->point_type());
		response->set_serie_id(result);
		providerFactory.Release(provider);
	}
	catch (const std::exception &e)
	{
		providerFactory.Release();
		std::cerr << e.what() << '\n';
	}

	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::StoreMetadata(::grpc::ServerContext *context, const ::volue::mesh::protobuf::StoreMetadataRequest *request, ::google::protobuf::Empty *response)
{
	try
	{
		auto provider = providerFactory.Get();

		Metadata meta{
			.serieId = request->serie_id(),
			.curveType = (uint8_t)(request->curve_type()),
			.delta = std::chrono::milliseconds(request->delta().delta()),
			.versionTS = request->version_ts(),
			.scopeType = (uint8_t)(request->scope_type()),
			.scopeId = (request->has_scope_id() ? std::optional<int>(request->scope_id()) : std::optional<int>(std::nullopt))};

		provider->StoreMetadata(meta);
		providerFactory.Release(provider);
	}
	catch (const std::exception &e)
	{
		providerFactory.Release();
		std::cerr << e.what() << '\n';
	}

	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::StorePoints(::grpc::ServerContext *context, const ::volue::mesh::protobuf::StorePointsRequest *request, ::volue::mesh::protobuf::StorePointsResponse *response)
{
	try
	{
		std::unique_ptr<StorePointData[]> pointsPtr(new StorePointData[request->points_size()]);

		for (int pos = 0; pos < request->points_size(); ++pos)
		{
			auto &point_in = request->points(pos);
			auto point_out = pointsPtr.get() + pos;

			point_out->privateFlags = point_in.private_flags();
			point_out->propagatingFlags = point_in.propagating_flags();
			point_out->value = point_in.value();

			point_out->dateTime.seconds = point_in.datetime().seconds();
			point_out->dateTime.nanos = point_in.datetime().nanos();
		}

		auto provider = providerFactory.Get();
		auto [rows_inserted, time_in_microsecs, rows_per_sec] =
			provider->StorePoints(request->serie_id(), request->version_ts(), (uint8_t)request->scope_type(),
								  (request->has_scope_id() ? std::optional<int>(request->scope_id()) : std::optional<int>(std::nullopt)),
								  request->points_size(), pointsPtr.get());

		response->set_rows_inserted(rows_inserted);
		response->set_time_in_microsecs(time_in_microsecs);
		response->set_rows_per_sec(rows_per_sec);

		providerFactory.Release(provider);
	}
	catch (const std::exception &e)
	{
		providerFactory.Release();
		std::cerr << e.what() << '\n';

		return ::grpc::Status(::grpc::StatusCode::ABORTED, e.what());
	}

	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::GetNextDataSetTS(::grpc::ServerContext *context, const ::google::protobuf::Empty *request, ::volue::mesh::protobuf::GetNextDataSetTSResponse *response)
{
	try
	{
		auto provider = providerFactory.Get();
		auto result = provider->GetNextVersionTS();
		response->set_next_ts(result);
		providerFactory.Release(provider);
	}
	catch (const std::exception &e)
	{
		providerFactory.Release();
		std::cerr << e.what() << '\n';
	}

	return Status::OK;
}

::grpc::Status TimeSeriesStorageService::ReadPoints(::grpc::ServerContext *context, const ::volue::mesh::protobuf::ReadPointsRequest *request, ::volue::mesh::protobuf::ReadPointsResponse *response)
{
	try
	{
		int serieId = request->serie_id();
		std::optional<timestamp> dtFrom;
		std::optional<timestamp> dtTo;
		std::optional<bool> includeEndpoint;

		if (request->has_datetimefrom())
		{
			// Seconds from unix epoch
			dtFrom = timestamp{.seconds = request->datetimefrom().seconds(), .nanos = request->datetimefrom().nanos()};
			dtTo = timestamp{.seconds = request->datetimeto().seconds(), .nanos = request->datetimeto().nanos()};

			if (request->has_includingendpoint())
				includeEndpoint = request->includingendpoint();
		}

		auto assignData = [&](timestamp ts, double value, int privateFlags, int propagatingFlags, int versionTS, uint8_t scope_type, std::optional<int> &scope_id)
		{
			auto pointData = response->add_points();

			auto dt = new google::protobuf::Timestamp();
			dt->set_seconds(ts.seconds);
			dt->set_nanos(ts.nanos);
			pointData->set_allocated_datetime(dt);

			pointData->set_value(value);
			pointData->set_private_flags(privateFlags);
			pointData->set_propagating_flags(propagatingFlags);
			pointData->set_version_ts(versionTS);
			pointData->set_scope_type(static_cast<volue::mesh::protobuf::ScopeType>(scope_type));

			if (scope_id.has_value())
				pointData->set_scope_id(scope_id.value());
		};

		auto provider = providerFactory.Get();
		auto [rows_read, time_in_microsecs, rows_per_sec] = provider->ReadPoints(serieId, dtFrom, dtTo, includeEndpoint, assignData);

		response->set_rows_read(rows_read);
		response->set_time_in_microsecs(time_in_microsecs);
		response->set_rows_per_sec(rows_per_sec);

		providerFactory.Release(provider);
	}
	catch (const std::exception &e)
	{
		providerFactory.Release();
		std::cerr << e.what() << '\n';
	}

	return Status::OK;
}