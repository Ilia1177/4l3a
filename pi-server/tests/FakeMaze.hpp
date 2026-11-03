#ifndef FAKE_MAZE_HPP
#define FAKE_MAZE_HPP

#include <string>

#include "IMaze.hpp"

class FakeMaze : public IMaze {
   public:
    void print_code() override { ++printCodeCalls; }

    std::string get_code() override {
        ++getCodeCalls;
        return code;
    }

    bool verify_pass(const std::string& submittedCode) override {
        ++verifyPassCalls;
        lastSubmittedCode = submittedCode;
        return verifyResult;
    }

    void game_over(const std::string& message) override {
        ++gameOverCalls;
        lastGameOverMessage = message;
    }

    void init() override { ++initCalls; }

    void play_game() override { ++playGameCalls; }

    int printCodeCalls = 0;
    int getCodeCalls = 0;
    int verifyPassCalls = 0;
    int gameOverCalls = 0;
    int initCalls = 0;
    int playGameCalls = 0;
    bool verifyResult = false;
    std::string code = "secret";
    std::string lastSubmittedCode;
    std::string lastGameOverMessage;
};

#endif  // FAKE_MAZE_HPP
