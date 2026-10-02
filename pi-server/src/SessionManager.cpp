#include "SessionManager.hpp"
#include <random>
#include <sstream>
#include <iostream>

std::optional<std::string> SessionManager::tryJoin() {
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();

    if (!token_.empty()) {
        return std::nullopt; // someone else is already playing
    }

    token_ = generateToken();
    lastActivity_ = std::chrono::steady_clock::now();
    return token_;
}

bool SessionManager::validate(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();

    if (token.empty() || token != token_) return false;
    lastActivity_ = std::chrono::steady_clock::now();
    return true;
}

void SessionManager::release(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (token == token_) {
        token_.clear();
    }
}

void SessionManager::setTimeout(int seconds) {
    std::lock_guard<std::mutex> lock(mtx_);
    kTimeoutSeconds_ = seconds;
}

void SessionManager::reapIfExpired() {
    if (token_.empty()) return;
    auto idle = std::chrono::steady_clock::now() - lastActivity_;
    if (idle > std::chrono::seconds(kTimeoutSeconds_)) {
        token_.clear(); // previous player went idle/disappeared
    }
}

std::string SessionManager::generateToken() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(gen);
    return oss.str();
}