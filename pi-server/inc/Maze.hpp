#ifndef MAZE_HPP
#define MAZE_HPP
#include "Minitel1B_Hard.h"

class Maze {
public:
    Maze(Minitel* minitel);
    ~Maze(void);

	void print_code();
    void init();
    void enter();
    bool verify_pass(std::string code);
    void set_pass(std::string code);
    int getLevel() const;
    void nextLevel();

private:
    size_t  _level;
    Minitel* _minitel;
    std::string _passcode;
};

#endif // MAZE_HPP
