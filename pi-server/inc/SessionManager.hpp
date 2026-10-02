#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <optional>
#include <string>
#include <mutex>
#include <chrono>

class SessionManager {
public:
    // Try to claim the session. Returns a token on success, nullopt if busy.
    std::optional<std::string> tryJoin();

    // Check a request's token is the current owner; refreshes activity if valid.
    bool validate(const std::string& token);

    // Explicit release (e.g. user clicks "leave" or closes the tab cleanly).
    void release(const std::string& token);

    // Set timeout in seconds (default 60)
    void setTimeout(int seconds);

private:
    std::mutex mtx_;
    std::string token_;
    std::chrono::steady_clock::time_point lastActivity_;
    int kTimeoutSeconds_;

    void reapIfExpired();

    std::string generateToken();
};

#endif // SESSION_MANAGER_HPP