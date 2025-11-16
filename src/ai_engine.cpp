#include "ai_engine.h"
#include "explorer.h"
#include <stdexcept>

#include <ai/openai.h>

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

    Engine eng{.client = std::move(client), .model = model, .api_key = api_key};
    return eng;
}
} // namespace pg_ask::ai_engine