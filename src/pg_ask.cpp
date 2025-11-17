extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/varlena.h"
#include "varatt.h"
#include "utils/guc.h"
}
#include <string>
#include "explorer.h"
#include "ai_engine.h"

// Configuration variables
static char* pg_ask_model = nullptr;
static char* pg_ask_endpoint = nullptr;

// Mandatory module magic

extern "C" {
PG_MODULE_MAGIC;

// Module load callback to define custom GUC variables
void _PG_init(void);

void _PG_init(void) {
    // Define pg_ask.model configuration variable
    DefineCustomStringVariable("pg_ask.model", "AI model to use for SQL generation (e.g., 'openai/gpt-4', 'openai/gpt-3.5-turbo')",
                               "Defaults to OpenAI's default model (gpt-4o) if not set", &pg_ask_model,
                               "", // Empty default - ai_engine will use OpenAI default
                               PGC_USERSET, 0, nullptr, nullptr, nullptr);

    // Define pg_ask.endpoint configuration variable
    DefineCustomStringVariable("pg_ask.endpoint",
                               "API endpoint for the AI provider (e.g., 'https://api.openai.com/v1', 'https://api.groq.com/openai')",
                               "Defaults to OpenAI endpoint (https://api.openai.com/v1) if not set", &pg_ask_endpoint,
                               "", // Empty default - ai_engine will use OpenAI endpoint
                               PGC_USERSET, 0, nullptr, nullptr, nullptr);
}
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
    } catch (...) {
        ereport(ERROR, (errmsg("Unknown error during catalog inspection/formatting")));
    }

    try {
        // Read configuration from PostgreSQL GUC variables
        // Pass empty strings to ai_engine if not configured - it will use OpenAI defaults
        std::string model = pg_ask_model ? std::string(pg_ask_model) : "";
        std::string endpoint = pg_ask_endpoint ? std::string(pg_ask_endpoint) : "";

        auto eng = pg_ask::ai_engine::make_engine("", model, endpoint);
        std::string generated_sql = pg_ask::ai_engine::generate_sql(eng, prompt, userQuery);
        PG_RETURN_TEXT_P(cstring_to_text(generated_sql.c_str()));
    } catch (const std::exception& e) {
        ereport(ERROR, (errmsg("AI integration error: %s", e.what())));
    } catch (...) {
        ereport(ERROR, (errmsg("Unknown error during AI integration")));
        PG_RETURN_NULL();
    }
}
}