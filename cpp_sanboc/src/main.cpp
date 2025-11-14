#include <iostream>
#include <ai/openai.h>

int main() {

    const char *api_key = std::getenv("API_KEY"); // Replace with your actual API key or set it in the environment variable
    auto client = ai::openai::create_client(
        api_key,
        "https://api.groq.com/openai/"
    );

    ai::GenerateOptions opts;
    opts.model = "llama-3.3-70b-versatile";
    opts.system = "You are a friendly assistant!";
    opts.prompt = "Why is the sky blue?";
    auto result  = client.generate_text(opts);
    if(result) {
        std::cout << result.text << std::endl;
    } else {
        std::cerr << "Error: " << result.error_message() << std::endl;
    }

    return 0;
}
