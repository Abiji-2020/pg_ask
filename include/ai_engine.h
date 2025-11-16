#pragma once

#include <string>
#include <ai/types/client.h>

namespace pg_ask::ai_engine {

struct Engine {
    ai::Client client;
    std::string model;
    std::string api_key;
};

Engine make_engine(const std::string& api_key, const std::string& model, const std::string& base_url);

std::string build_prompt(const std::string& user_query, const std::string& database_schema);

std::string generate_sql(Engine& eng, const std::string& prompt);
} // namespace pg_ask::ai_engine