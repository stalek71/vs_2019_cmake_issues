﻿-- DO $$ BEGIN
--  CALL info.ensure_schema_version(1);
-- END $$;

-- version_ts column is to mark a proper data generation, "char" type is a special type taking 1 byte only

-- Timeseries master ID (to be used externally - on generated calculations)
-- DROP TABLE ts.serie;
CREATE TABLE ts.serie(
    name        VARCHAR PRIMARY KEY,  -- Model should use timeseries name instead of guid (so it's id independent)
    point_type  "char"  NOT NULL,     -- Single/P1D (1), P2D (2), P3D (3) and so on
    id          INT     GENERATED BY DEFAULT AS IDENTITY
);

-- INSERT INTO ts.serie (name, point_type, id) VALUES ('TS-Oslo',1::"char",DEFAULT) RETURNING id;


-- DROP TABLE ts.metadata;
CREATE TABLE ts.metadata(
    serie_id    INT       NOT NULL,
    curve_type  "char"    NOT NULL, -- StaircaseStartOfStep(1), PiecewiseLinear(2)
    delta       INTERVAL  NOT NULL, -- When delta is 0 it's BP
    version_ts  INT       NOT NULL, -- A column to mark the proper data generation
    scope_type  "char"    NOT NULL, -- Global(0), User(50), Session(100)
    scope_id    INT       NULL,     -- In the case of a user it should come from users dictionary (maybe session also?)

    PRIMARY KEY (serie_id, scope_type, scope_id, version_ts)
);

-- A table holding normal data points (Single/1D)
-- DROP TABLE ts.point;
CREATE TABLE ts.point(
    serie_id          INT               NOT NULL,
    datetime          TIMESTAMPTZ       NOT NULL,
    value             DOUBLE PRECISION  NOT NULL,
    private_flags     INT               NOT NULL,
    propagating_flags INT               NOT NULL,
    version_ts        INT               NOT NULL, -- A column to mark the proper data generation
    scope_type        "char"            NOT NULL, -- Global(0), User(50), Session(100)
    scope_id          INT               NULL,     -- In the case of a user it should come from users dictionary (maybe session also?)

    PRIMARY KEY (serie_id, datetime, scope_type, scope_id, version_ts)
);