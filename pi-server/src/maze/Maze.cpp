#include "Maze.hpp"
#include <random>

Maze::Maze(Minitel* m): _level(0), _minitel(m), _passcode("") 
{
	if(init_minitel() < 0) {
		throw std::runtime_error("Minitel init fail");
	}
}

Maze::~Maze(){ if (_minitel) delete _minitel; }

void Maze::set_pass(std::string code)
{
	_passcode = code;
}

void Maze::game_over(std::string msg)
{
	_minitel->clearScreen();
	_minitel->println(msg);
	_minitel->println("YOU LOSE");
	_level = 1;
}

void Maze::enter() {
	switch(_level) {
		case 0:
			return;
		default:
			std::string msg = "you are at level: " + std::to_string(_level);
			_minitel->clearScreen();
			_minitel->println(msg);
	}
	nextLevel();
}

void Maze::print_code() {
	_minitel->clearScreen();
	_minitel->println(_passcode);
}

int Maze::init_minitel() 
{
    Minitel* machine = _minitel;
    byte     reponse;

    machine->newScreen();
    reponse = machine->echo(false);
	if (reponse == 0x44) {
		_minitel->smallMode();
  		_minitel->extendedKeyboard();  // Clavier étendu
		return 0;
	}
	return -1;
}

void Maze::init() 
{
	std::string code = "MAZE001";
	set_pass(code);
	_level = 1;
}

bool Maze::verify_pass(std::string code) {
	if (code != _passcode) {
		return false;
	}
	return true;
}

int Maze::getLevel() const {
    return static_cast<int>(_level);
}

void Maze::nextLevel() {
    _level++;
}
