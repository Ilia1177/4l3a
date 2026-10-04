#include "SessionManager.hpp"
#include <random>
#include <sstream>
#include <iostream>

void log_line(const std::string& line);

SessionManager::SessionManager(void): kTimeoutSeconds_(60) {};

std::optional<std::string> SessionManager::tryJoin() {
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();
    if (!token_.empty()) {
        return std::nullopt; // someone else is already playing
    }
    token_ = generateToken();
	log_line("Token generated: " + token_ + "\n");
    lastActivity_ = std::chrono::steady_clock::now();
	sessionStart_ = lastActivity_;
    return token_;
}

bool SessionManager::validate(const std::string& token) 
{
    std::lock_guard<std::mutex> lock(mtx_);
    reapIfExpired();
    if (token.empty() || token != token_) {
		std::ostringstream oss;
		oss << "[session] rejected. got=" << token << " expected=" << token_ << "\n";
        log_line(oss.str());
        return false;
    }
    lastActivity_ = std::chrono::steady_clock::now();
    return true;
}

void SessionManager::release(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (token == token_) {
		log_line("release() called with token=" + token + " current=" + token_);
        token_.clear();
    }
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
    if (token_.empty()) return 0;
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - sessionStart_).count();
    int remaining = kTimeoutSeconds_ - static_cast<int>(elapsed);
    return remaining > 0 ? remaining : 0;
}

void SessionManager::reapIfExpired() {
    if (token_.empty()) return;
    auto idle = std::chrono::steady_clock::now() - lastActivity_;
    if (idle > std::chrono::seconds(kTimeoutSeconds_)) {
        token_.clear();
        log_line("Reap TOKEN");
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
