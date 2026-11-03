#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>

class SessionManager {
   public:
    using Clock = std::chrono::steady_clock;
    using NowFunction = std::function<Clock::time_point()>;

    SessionManager() noexcept;
    explicit SessionManager(NowFunction now);

    std::optional<std::string> tryJoin();
    bool validate(const std::string& token);
    bool is_valid();
    void release(const std::string& token);
    void invalidate();
    void setTimeout(int seconds);
    void addTimeout(int seconds);
    int get_time_left();

   private:
    std::mutex mtx_;
    std::string token_;
    Clock::time_point lastActivity_;
    Clock::time_point sessionStart_;
    int kTimeoutSeconds_;
    NowFunction now_;

    void reapIfExpired();
    [[nodiscard]] Clock::time_point currentTime() const;
    std::string generateToken();
};

#endif  // SESSION_MANAGER_HPP
