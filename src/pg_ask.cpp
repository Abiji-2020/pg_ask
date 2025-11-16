extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/varlena.h"
#include "varatt.h"
}
#include <string>
#include "explorer.h"
#include "ai_engine.h"

// Mandatory module magic

extern "C" {
PG_MODULE_MAGIC;
// Mandatory function info macro — declares the function with C++ linkage
PG_FUNCTION_INFO_V1(pg_gen_query);

Datum pg_gen_query(PG_FUNCTION_ARGS) {
    text* input = PG_GETARG_TEXT_PP(0);

    std::string userQuery(VARDATA_ANY(input), VARSIZE_ANY_EXHDR(input));
    std::string database_schema;
    std::string formatted_schema;
    std::string prompt;

    try {
        database_schema = buildDatabaseMap();
        formatted_schema = formatSchema(database_schema);
        prompt = pg_ask::ai_engine::build_prompt(formatted_schema);
    } catch (const std::exception& e) {
        /* Surface catalog/formatting errors as a PostgreSQL ERROR */
        ereport(ERROR, (errmsg("Catalog inspection/formatting error: %s", e.what())));
        PG_RETURN_NULL();
    } catch (...) {
        ereport(ERROR, (errmsg("Unknown error during catalog inspection/formatting")));
        PG_RETURN_NULL();
    }

    try {
        auto eng = pg_ask::ai_engine::make_engine("", "openai/gpt-oss-20b", "https://api.groq.com/openai");
        std::string generated_sql = pg_ask::ai_engine::generate_sql(eng, prompt, userQuery);
        PG_RETURN_TEXT_P(cstring_to_text(generated_sql.c_str()));
    } catch (const std::exception& e) {
        ereport(ERROR, (errmsg("AI integration error: %s", e.what())));
    } catch (...) {
        PG_RETURN_NULL();
    }
}
}