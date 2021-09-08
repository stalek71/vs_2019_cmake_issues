-- Core stuff

CREATE OR REPLACE PROCEDURE info.ensure_schema_version(version_nr INT)
LANGUAGE PLPGSQL
AS $$

DECLARE
  schema_ver int := 0;

BEGIN

  SELECT schema_version 
  FROM info.dbversion
  INTO schema_ver;

  IF schema_ver<>version_nr THEN
    RAISE EXCEPTION 'Wrong schema version...';
  END IF;

end $$;