#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Logger.hpp"
#include "WordSanitizer.hpp"
#include "httplib.h"

namespace {

std::string request_word(uint32_t seed) {
    httplib::Client client("127.0.0.1", 11434);
    client.set_read_timeout(30, 0);

    nlohmann::json request = {{"model", "gemma3:270m"},
                              {"prompt",
                               "Generate one random common lowercase English word. "
                               "Output only the word, with no spaces, no punctuation, "
                               "no quotes and no explanation."},
                              {"stream", false},
                              {"options",
                               {{"temperature", 0.9},
                                {"top_k", 40},
                                {"top_p", 0.9},
                                {"repeat_penalty", 1.1},
                                {"seed", seed},
                                {"num_predict", 8},
                                {"stop", {"\n", " ", ".", ","}}}}};

    auto response = client.Post("/api/generate", request.dump(), "application/json");

    if (!response) {
        throw std::runtime_error("Cannot connect to Ollama");
    }

    if (response->status != 200) {
        throw std::runtime_error("Ollama HTTP error");
    }

    nlohmann::json result = nlohmann::json::parse(response->body);

    if (!result.contains("response") || !result["response"].is_string()) {
        throw std::runtime_error("Invalid Ollama response");
    }

    return result["response"].get<std::string>();
}

std::string random_hex_word() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;

    std::ostringstream oss;
    oss << std::hex << dist(gen);
    return oss.str();
}

}  // namespace

std::string get_random_word() {
    std::random_device rd;
    std::uniform_int_distribution<uint32_t> seed_dist;

    for (int attempt = 0; attempt < 3; ++attempt) {
        try {
            std::string word = sanitize_word(request_word(seed_dist(rd)));
            if (!word.empty()) {
                return word;
            }
        } catch (const std::exception& error) {
            log_line(std::string("Ollama request failed: ") + error.what());
        }
    }

    return random_hex_word();
}
