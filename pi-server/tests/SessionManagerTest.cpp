#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "SessionManager.hpp"

namespace {

struct FakeClock {
    SessionManager::Clock::time_point now{};
};

SessionManager::NowFunction readClock(FakeClock& clock) {
    return [&clock] { return clock.now; };
}

void advance(FakeClock& clock, int seconds) { clock.now += std::chrono::seconds(seconds); }

std::string join(SessionManager& session) {
    auto token = session.tryJoin();
    EXPECT_TRUE(token.has_value());
    return token.value_or("");
}

}  // namespace

TEST(SessionManagerTest, TryJoinCreatesSession) {
    FakeClock clock;
    SessionManager session(readClock(clock));

    auto token = session.tryJoin();

    ASSERT_TRUE(token.has_value());
    EXPECT_FALSE(token->empty());
    EXPECT_TRUE(session.is_valid());
}

TEST(SessionManagerTest, TryJoinFailsWhileSessionIsActive) {
    FakeClock clock;
    SessionManager session(readClock(clock));

    join(session);

    EXPECT_FALSE(session.tryJoin().has_value());
}

TEST(SessionManagerTest, ValidateRequiresMatchingToken) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    const std::string token = join(session);

    EXPECT_TRUE(session.validate(token));
    EXPECT_FALSE(session.validate("wrong-token"));
    EXPECT_FALSE(session.validate(""));
    EXPECT_TRUE(session.is_valid());
}

TEST(SessionManagerTest, IsValidExpiresAfterIdleTimeout) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    join(session);

    advance(clock, 61);

    EXPECT_FALSE(session.is_valid());
}

TEST(SessionManagerTest, ExpiredSessionCanBeJoinedAgain) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    const std::string firstToken = join(session);

    advance(clock, 61);
    EXPECT_FALSE(session.is_valid());

    auto secondToken = session.tryJoin();
    ASSERT_TRUE(secondToken.has_value());
    EXPECT_NE(*secondToken, firstToken);
}

TEST(SessionManagerTest, ValidateRefreshesIdleTimeout) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    session.setTimeout(100);
    const std::string token = join(session);

    advance(clock, 40);
    EXPECT_TRUE(session.validate(token));
    advance(clock, 40);

    EXPECT_TRUE(session.is_valid());
    EXPECT_EQ(session.get_time_left(), 20);
}

TEST(SessionManagerTest, GetTimeLeftUsesSessionStart) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    session.setTimeout(10);
    join(session);

    advance(clock, 4);

    EXPECT_EQ(session.get_time_left(), 6);
}

TEST(SessionManagerTest, SetTimeoutChangesRemainingTime) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    session.setTimeout(10);
    join(session);
    advance(clock, 4);

    session.setTimeout(20);

    EXPECT_EQ(session.get_time_left(), 16);
}

TEST(SessionManagerTest, AddTimeoutExtendsRemainingTime) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    session.setTimeout(10);
    join(session);

    session.addTimeout(5);
    advance(clock, 10);

    EXPECT_EQ(session.get_time_left(), 5);
}

TEST(SessionManagerTest, InvalidateClearsSessionAndResetsTimeout) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    session.setTimeout(10);
    join(session);

    session.invalidate();

    EXPECT_FALSE(session.is_valid());
    join(session);
    EXPECT_EQ(session.get_time_left(), 60);
}

TEST(SessionManagerTest, ReleaseOnlyClearsMatchingToken) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    const std::string token = join(session);

    session.release("wrong-token");
    EXPECT_TRUE(session.is_valid());

    session.release(token);
    EXPECT_FALSE(session.is_valid());
}

TEST(SessionManagerTest, ConcurrentJoinOnlyOneSucceeds) {
    FakeClock clock;
    SessionManager session(readClock(clock));
    std::atomic<int> successes{0};
    std::vector<std::thread> threads;
    constexpr int kThreadCount = 8;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&session, &successes] {
            if (session.tryJoin().has_value()) {
                successes.fetch_add(1);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successes.load(), 1);
}
