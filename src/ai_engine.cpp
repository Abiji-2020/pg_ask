#include "ai_engine.h"
#include "explorer.h"
#include <stdexcept>

#include <ai/openai.h>
#include "nlohmann/json.hpp"
#include <ai/types/generate_options.h>

namespace pg_ask::ai_engine {
Engine make_engine(const std::string& api_key, const std::string& model, const std::string& base_url) {
    if (api_key.empty()) {
        const char* env = std::getenv("PG_ASK_AI_KEY");
        if (!env) {
            throw std::runtime_error("API Key is required to process the queries");
        }
        return make_engine(env, model, base_url);
    }

    std::string final_model = model.empty() ? ai::openai::models::kDefaultModel : model;
    ai::Client client;
    if (base_url.empty()) {
        client = ai::openai::create_client(api_key);
    } else {
        client = ai::openai::create_client(api_key, base_url);
    }

    Engine eng{.client = std::move(client), .model = final_model, .api_key = api_key};
    return eng;
}

std::string build_prompt(const std::string& database_schema) {
    std::string p;
    p.reserve(database_schema.size() + 2048);

    p += "You are a senior PostgreSQL engineer. ";
    p += "Your task is to convert natural language requests into SQL queries, ";
    p += "strictly using ONLY the tables and columns provided in the schema.\n\n";

    p += "The schema format is:\n";
    p += "SCHEMA.TABLE_NAME(COLUMN_NAME DATA_TYPE)\n\n";

    // --- Few-shot exemplars ---
    p += "EXAMPLE 1:\n";
    p += "Schema:\n";
    p += "demo.users(id integer, name text, age integer)\n";
    p += "demo.orders(id integer, user_id integer, amount numeric)\n";
    p += "User request: show me all users\n";
    p += "SQL: SELECT id, name, age FROM demo.users;\n\n";

    p += "EXAMPLE 2:\n";
    p += "Schema:\n";
    p += "shop.products(id integer, title text, price numeric)\n";
    p += "shop.sales(id integer, product_id integer, quantity integer)\n";
    p += "User request: list all product titles that have been sold\n";
    p += "SQL: SELECT p.title FROM shop.products p JOIN shop.sales s ON p.id = s.product_id;\n\n";

    p += "EXAMPLE 3:\n";
    p += "Schema:\n";
    p += "analytics.logs(id integer, created_at timestamp, message text)\n";
    p += "User request: what are the logs?\n";
    p += "SQL: SELECT id, created_at, message FROM analytics.logs;\n\n";

    // --- Ground truth schema ---
    p += "AVAILABLE SCHEMA:\n";
    p += database_schema;
    p += "\n\n";

    // --- Rules ---
    p += "RULES:\n";
    p += "- Output exactly one SQL query.\n";
    p += "- No explanations. No commentary. No markdown.\n";
    p += "- Do not invent tables or columns not in the schema.\n";
    p += "- Prefer explicit column names over SELECT *.\n";
    p += "- Use schema-qualified names when needed.\n";
    p += "- If the question cannot be answered, return: SELECT NULL;\n\n";

    p += "Now generate the SQL query for the user's request.";

    return p;
}

std::string generate_sql(Engine& eng, const std::string& prompt, const std::string& user_query) {

    ai::GenerateOptions options;
    options.model = eng.model;
    options.temperature = 0.0;
    options.system = prompt;
    options.response_format =
        nlohmann::json{{"type", "json_schema"},
                       {"json_schema",
                        {{"name", "sql_response"},
                         {"schema", {{"type", "object"}, {"properties", {{"sql", {{"type", "string"}}}}}, {"required", {"sql"}}}}}}};

    options.prompt = user_query;
    ai::GenerateResult result = eng.client.generate_text(options);
    if (result.error) {
        throw std::runtime_error("Error from the LLM Client: " + result.error_message());
    }
    auto sql_json = nlohmann::json::parse(result.text);
    std::string sql = sql_json["sql"].get<std::string>();
    return sql;
}

} // namespace pg_ask::ai_engine