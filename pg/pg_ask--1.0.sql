-- pg_ask 1.0 extension
CREATE OR REPLACE FUNCTION pg_gen_query(query text)
RETURNS text
AS 'pg_ask', 'pg_gen_query'
LANGUAGE C STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION pg_gen_execute(user_query text)
RETURNS refcursor -- Returns a cursor name
LANGUAGE plpgsql AS
$$
DECLARE
    generated_sql text;
    result_cursor refcursor := 'ai_query_result'; -- A name for the cursor
BEGIN
    generated_sql := pg_gen_query(user_query);
    IF generated_sql IS NULL OR generated_sql = '' THEN
        RAISE EXCEPTION 'pg_gen_query returned empty SQL';
    END IF;

    -- Open a cursor for the dynamic query
    OPEN result_cursor FOR EXECUTE generated_sql;
    
    -- Return the name of the cursor
    RETURN result_cursor;
END;
$$;
