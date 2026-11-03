#ifndef MAZE_HPP
#define MAZE_HPP

#include <atomic>
#include <mutex>
#include <string>

#include "IMaze.hpp"
#include "MazeCode.hpp"
#include "Minitel1B_Hard.h"
#include "SessionManager.hpp"

class Maze : public IMaze {
   public:
    Maze();
    ~Maze(void) override;

    void wakeup();
    void level_1();
    void level_2();
    void level_3();
    void level_4();
    int init_minitel();
    void game_over(const std::string& message) override;
    void print_code() override;
    void init() override;
    void play_game() override;
    bool verify_pass(const std::string& code) override;
    std::string get_code() override;
    int getLevel() const;
    void nextLevel();
    void update_play_time(SessionManager& session);

   private:
    int _time_left;
    std::atomic<int> _level;
    HardwareSerial _serial;
    Minitel* _minitel;
    MazeCode _code;
    mutable std::mutex _mtx;
};

#endif  // MAZE_HPP
