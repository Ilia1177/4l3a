#include <gtest/gtest.h>

#include <string>

#include "FakeMaze.hpp"
#include "MinitelController.hpp"
#include "SessionManager.hpp"

namespace {

std::string extractToken(const std::string& body) {
    const std::string key = "\"token\":\"";
    const auto begin = body.find(key);
    if (begin == std::string::npos) {
        return "";
    }
    const auto valueBegin = begin + key.size();
    const auto end = body.find('"', valueBegin);
    if (end == std::string::npos) {
        return "";
    }
    return body.substr(valueBegin, end - valueBegin);
}

httplib::Request requestWithToken(const std::string& token) {
    httplib::Request req;
    req.set_header("X-Session-Token", token);
    return req;
}

}  // namespace

class MinitelControllerTest : public ::testing::Test {
   protected:
    std::string join() {
        httplib::Request req;
        httplib::Response res;
        controller.join(req, res);
        EXPECT_EQ(res.status, 200);
        return extractToken(res.body);
    }

    SessionManager session;
    FakeMaze maze;
    MinitelController controller{session, maze};
};

TEST_F(MinitelControllerTest, JoinReturnsToken) {
    httplib::Request req;
    httplib::Response res;

    controller.join(req, res);

    EXPECT_EQ(res.status, 200);
    EXPECT_NE(extractToken(res.body), "");
    EXPECT_TRUE(session.is_valid());
}

TEST_F(MinitelControllerTest, SecondJoinReturnsConflict) {
    join();
    httplib::Request req;
    httplib::Response res;

    controller.join(req, res);

    EXPECT_EQ(res.status, 409);
    EXPECT_EQ(res.body, "{\"error\":\"session busy\"}");
}

TEST_F(MinitelControllerTest, ValidSessionRequiresToken) {
    httplib::Request noTokenReq;
    httplib::Response noTokenRes;
    controller.validSession(noTokenReq, noTokenRes);
    EXPECT_EQ(noTokenRes.status, 401);

    const std::string token = join();
    auto req = requestWithToken(token);
    httplib::Response res;
    controller.validSession(req, res);
    EXPECT_EQ(res.status, 200);
}

TEST_F(MinitelControllerTest, LeaveReleasesSession) {
    const std::string token = join();
    auto req = requestWithToken(token);
    httplib::Response res;

    controller.leave(req, res);

    EXPECT_EQ(res.status, 200);
    EXPECT_FALSE(session.is_valid());
}

TEST_F(MinitelControllerTest, LeaveWithWrongTokenDoesNotReleaseSession) {
    join();
    auto req = requestWithToken("wrong-token");
    httplib::Response res;

    controller.leave(req, res);

    EXPECT_EQ(res.status, 200);
    EXPECT_TRUE(session.is_valid());
}

TEST_F(MinitelControllerTest, PrintCodeRequiresValidSession) {
    httplib::Request noTokenReq;
    httplib::Response noTokenRes;
    controller.printcode(noTokenReq, noTokenRes);
    EXPECT_EQ(noTokenRes.status, 403);
    EXPECT_EQ(maze.printCodeCalls, 0);

    const std::string token = join();
    auto req = requestWithToken(token);
    httplib::Response res;
    controller.printcode(req, res);

    EXPECT_EQ(res.status, 200);
    EXPECT_EQ(maze.printCodeCalls, 1);
    EXPECT_EQ(maze.getCodeCalls, 1);
}

TEST_F(MinitelControllerTest, SubmitCodeRequiresValidSession) {
    httplib::Request req;
    httplib::Response res;

    controller.submitCode(req, res);

    EXPECT_EQ(res.status, 403);
    EXPECT_EQ(maze.verifyPassCalls, 0);
}

TEST_F(MinitelControllerTest, SubmitCodeRequiresCodeParameter) {
    const std::string token = join();
    auto req = requestWithToken(token);
    httplib::Response res;

    controller.submitCode(req, res);

    EXPECT_EQ(res.status, 400);
    EXPECT_EQ(maze.verifyPassCalls, 0);
}

TEST_F(MinitelControllerTest, SubmitWrongCodeCallsGameOverThenInit) {
    const std::string token = join();
    auto req = requestWithToken(token);
    req.params.emplace("code", "wrong");
    httplib::Response res;

    controller.submitCode(req, res);

    EXPECT_EQ(res.status, 401);
    EXPECT_EQ(maze.verifyPassCalls, 1);
    EXPECT_EQ(maze.lastSubmittedCode, "wrong");
    EXPECT_EQ(maze.gameOverCalls, 1);
    EXPECT_EQ(maze.initCalls, 1);
    EXPECT_EQ(maze.playGameCalls, 0);
    EXPECT_EQ(maze.lastGameOverMessage, "Wrong password...");
}

TEST_F(MinitelControllerTest, SubmitCorrectCodePlaysGame) {
    maze.verifyResult = true;
    const std::string token = join();
    auto req = requestWithToken(token);
    req.params.emplace("code", "secret");
    httplib::Response res;

    controller.submitCode(req, res);

    EXPECT_EQ(res.status, 200);
    EXPECT_EQ(maze.verifyPassCalls, 1);
    EXPECT_EQ(maze.lastSubmittedCode, "secret");
    EXPECT_EQ(maze.playGameCalls, 1);
    EXPECT_EQ(maze.gameOverCalls, 0);
    EXPECT_EQ(maze.initCalls, 0);
}
