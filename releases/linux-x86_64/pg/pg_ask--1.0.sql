-- pg_ask 1.0 extension
CREATE OR REPLACE FUNCTION pg_gen_query(query text)
RETURNS text
AS 'pg_ask', 'pg_gen_query'
LANGUAGE C STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION pg_execute_query(query text)
RETURNS text
AS 'pg_ask', 'pg_execute_query'
LANGUAGE C STRICT PARALLEL SAFE;
