#ifndef MAZE_HPP
#define MAZE_HPP
#include <atomic>
#include <string>
#include <mutex>

#include "Minitel1B_Hard.h"
#include "SessionManager.hpp"

class Maze {
public:
    Maze();
    ~Maze(void);

	void level_1();
	void level_2();
	void level_3();
	void level_4();
	int init_minitel();
	void game_over(std::string c);
	void print_code();
    void init();
    void play_game();
    bool verify_pass(std::string code);
	std::string get_code();
    int getLevel() const;
    void nextLevel();
	void update_play_time(SessionManager& session);

private:
    void set_pass();
	int _time_left;
	std::atomic<int> _level;
    HardwareSerial _serial;
    Minitel* _minitel;
    std::string _passcode;
	mutable std::mutex _mtx;
};

#endif // MAZE_HPP
