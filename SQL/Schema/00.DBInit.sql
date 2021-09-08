-- Some initialization

-- DB info schema
CREATE SCHEMA info;

-- Dictionaries
CREATE SCHEMA dic;

-- TimeSeries schema (groups all timepoint related data)
CREATE SCHEMA ts;

-- Model schema
CREATE SCHEMA model;

-- DBVersion table
DROP TABLE IF EXISTS info.dbversion;
CREATE  TABLE info.DBVersion(
  schema_version INT,
  sp_version INT -- for stored procedures
);

-- Initial version info
INSERT INTO info.dbversion(schema_version, sp_version) VALUES(1, 1);

-- Sequence generator for data set versioning
DROP SEQUENCE IF EXISTS data_version_ts;
CREATE SEQUENCE IF NOT EXISTS data_version_ts AS INTEGER;
-- SELECT NEXTVAL('data_version_ts')