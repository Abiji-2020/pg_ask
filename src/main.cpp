#include <ai/openai.h>

#include <iostream>

int main() {
    const char* api_key = std::getenv("API_KEY");
    if (!api_key) {
        std::cerr << "Please set the API_KEY environment variable." << std::endl;
        return 1;
    }
    auto client = ai::openai::create_client(api_key, "https://api.groq.com/openai/");

    ai::GenerateOptions opts;
    opts.model = "llama-3.3-70b-versatile";
    opts.system = "You are a friendly assistant!";
    opts.prompt = "Why is the sky blue?";

    auto result = client.generate_text(opts);
    if (result) {
        std::cout << result.text << std::endl;
    } else {
        std::cerr << "Error: " << result.error_message() << std::endl;
    }

    return 0;
}
