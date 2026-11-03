#include "SessionManager.hpp"

#include <cstdint>
#include <random>
#include <sstream>
#include <utility>

void log_line(const std::string& line);

SessionManager::SessionManager() noexcept : kTimeoutSeconds_(60) {}

SessionManager::SessionManager(NowFunction now) : kTimeoutSeconds_(60), now_(std::move(now)) {}

std::optional<std::string> SessionManager::tryJoin() {
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();
    if (!token_.empty()) {
        return std::nullopt;
    }
    token_ = generateToken();
    log_line("join session -> Token generated: " + token_ + "\n");
    lastActivity_ = currentTime();
    sessionStart_ = lastActivity_;
    return token_;
}

bool SessionManager::validate(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();
    if (token.empty() || token != token_) {
        std::ostringstream oss;
        oss << "[session] rejected. got=" << token << " expected=" << token_ << "\n";
        log_line(oss.str());
        return false;
    }
    lastActivity_ = currentTime();
    return true;
}

void SessionManager::invalidate() {
    std::lock_guard<std::mutex> lock(mtx_);
    token_.clear();
    kTimeoutSeconds_ = 60;
}

bool SessionManager::is_valid() {
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();
    return !token_.empty();
}

void SessionManager::release(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (token == token_) {
        token_.clear();
        return;
    }
    log_line("release() called with a mismatched token");
}

void SessionManager::addTimeout(int seconds) {
    std::lock_guard<std::mutex> lock(mtx_);
    kTimeoutSeconds_ += seconds;
}

void SessionManager::setTimeout(int seconds) {
    std::lock_guard<std::mutex> lock(mtx_);
    kTimeoutSeconds_ = seconds;
}

int SessionManager::get_time_left() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (token_.empty()) {
        return 0;
    }
    auto elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime() - sessionStart_).count();
    int remaining = kTimeoutSeconds_ - static_cast<int>(elapsed);
    return remaining > 0 ? remaining : 0;
}

void SessionManager::reapIfExpired() {
    if (token_.empty()) {
        return;
    }
    auto idle = currentTime() - lastActivity_;
    if (idle > std::chrono::seconds(kTimeoutSeconds_)) {
        token_.clear();
        log_line("Reap TOKEN");
    }
}

SessionManager::Clock::time_point SessionManager::currentTime() const {
    return now_ ? now_() : Clock::now();
}

std::string SessionManager::generateToken() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(gen);
    return oss.str();
}
