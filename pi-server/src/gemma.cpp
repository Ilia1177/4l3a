#include "httplib.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

std::string get_random_word()
{
    httplib::Client client("127.0.0.1", 11434);
    client.set_read_timeout(30, 0);

    nlohmann::json request = {
        {"model", "gemma3:270m"},
        {"prompt",
            "Give me exactly one random English word. "
            "Output only the word, without punctuation."},
        {"stream", false},
        {"options", {
            {"temperature", 1.5},
            {"top_k", 100},
            {"top_p", 0.95},
            {"repeat_penalty", 1.2},
            {"seed", 0},
            {"num_predict", 20}
        }}
    };

    auto response = client.Post(
        "/api/generate",
        request.dump(),
        "application/json"
    );

    if (!response)
        throw std::runtime_error("Cannot connect to Ollama");

    if (response->status != 200)
        throw std::runtime_error("Ollama HTTP error");

    nlohmann::json result =
        nlohmann::json::parse(response->body);

    if (!result.contains("response") ||
        !result["response"].is_string())
        throw std::runtime_error("Invalid Ollama response");

    return result["response"].get<std::string>();
}
