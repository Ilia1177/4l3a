#include "WordSanitizer.hpp"

std::string sanitize_word(std::string_view raw) {
    const auto begin = raw.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos) {
        return "";
    }

    const auto end = raw.find_first_of(" \t\r\n", begin);
    std::string token(raw.substr(begin, end - begin));

    while (!token.empty() && (token.front() == '"' || token.front() == '\'')) {
        token.erase(token.begin());
    }
    while (!token.empty() && (token.back() == '"' || token.back() == '\'')) {
        token.pop_back();
    }

    std::string out;
    for (char c : token) {
        const auto value = static_cast<unsigned char>(c);
        if (value >= 33 && value <= 126) {
            out.push_back(static_cast<char>(value));
        }
    }

    if (out.size() > 24) {
        out.resize(24);
    }
    return out;
}
