-- pg_ask 1.0 extension

CREATE OR REPLACE FUNCTION pg_ask(query text)
RETURNS text
AS 'pg_ask', 'pg_gen_query'
LANGUAGE C STRICT PARALLEL SAFE;
