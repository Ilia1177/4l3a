#include "Maze.hpp"
#include <random>

Maze::Maze(Minitel* m): _level(0), _minitel(m), _passcode("") {}
Maze::~Maze(){ if (_minitel) delete _minitel; }

void Maze::set_pass(std::string code)
{
	_passcode = code;
}

void Maze::enter() {
	if (!_level) {
		return;
	}
	std::string msg = "you are at level: " + std::to_string(_level);
	_minitel->clearScreen();
	_minitel->println(msg);
}

void Maze::print_code() {
	_minitel->clearScreen();
	_minitel->println(_passcode);
}

void Maze::init() {
	std::string code = "MAZE001";
	set_pass(code);
	_level = 1;
}

bool Maze::verify_pass(std::string code) {
	if (code != _passcode) {
		return false;
	}
	_level = 1;
	return true;
}

int Maze::getLevel() const {
    return static_cast<int>(_level);
}

void Maze::nextLevel() {
    _level++;
}
