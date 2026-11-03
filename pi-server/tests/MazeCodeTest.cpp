#include <gtest/gtest.h>

#include <string>

#include "MazeCode.hpp"

TEST(MazeCodeTest, StartsEmptyUntilReset) {
    MazeCode code([] { return "alpha"; });

    EXPECT_EQ(code.value(), "");
    EXPECT_FALSE(code.verify("alpha"));
}

TEST(MazeCodeTest, ResetLoadsProviderValue) {
    MazeCode code([] { return "alpha"; });

    code.reset();

    EXPECT_EQ(code.value(), "alpha");
    EXPECT_TRUE(code.verify("alpha"));
}

TEST(MazeCodeTest, VerifyIsCaseSensitive) {
    MazeCode code([] { return "alpha"; });
    code.reset();

    EXPECT_FALSE(code.verify("Alpha"));
    EXPECT_FALSE(code.verify("ALPHA"));
}

TEST(MazeCodeTest, VerifyRejectsEmptyCode) {
    MazeCode code([] { return ""; });
    code.reset();

    EXPECT_FALSE(code.verify(""));
}

TEST(MazeCodeTest, ResetCallsProviderAgain) {
    int calls = 0;
    MazeCode code([&calls] {
        ++calls;
        return "value-" + std::to_string(calls);
    });

    code.reset();
    EXPECT_EQ(code.value(), "value-1");
    code.reset();
    EXPECT_EQ(code.value(), "value-2");
    EXPECT_EQ(calls, 2);
}
