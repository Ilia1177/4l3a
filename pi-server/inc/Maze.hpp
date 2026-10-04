#ifndef MAZE_HPP
#define MAZE_HPP
#include <atomic>
#include "Minitel1B_Hard.h"
#include "SessionManager.hpp"

class Maze {
public:
    Maze(Minitel* minitel);
    ~Maze(void);

	int init_minitel();
	void game_over(std::string c);
	void print_code();
    void init();
    void enter();
    bool verify_pass(std::string code);
    void set_pass();
    int getLevel() const;
    void nextLevel();
	void update_play_time(SessionManager& session);

private:
	int _time_left;
	std::atomic<int> _level;
    Minitel* _minitel;
    std::string _passcode;
	mutable std::mutex _mtx;
};

#endif // MAZE_HPP
