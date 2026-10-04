#include "Maze.hpp"
#include <random>

Maze::Maze(Minitel* m): _time_left(0), _level(0), _minitel(m), _passcode("") 
{
	if(init_minitel() < 0) {
		throw std::runtime_error("Minitel init fail");
	}
}

Maze::~Maze() { if (_minitel) delete _minitel; }

void Maze::update_play_time(SessionManager& session) {
	std::lock_guard<std::mutex> lock(_mtx);
	std::string time = std::to_string(session.get_time_left());
	_minitel->println00(time);
}

void Maze::set_pass(std::string code)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_passcode = code;
}

void Maze::game_over(std::string msg)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_minitel->clearScreen();
	_minitel->println(msg);
	_minitel->println("YOU LOSE");
	_level = 1;
}

void Maze::enter() {
	std::lock_guard<std::mutex> lock(_mtx);
	switch(_level) {
		case 0:
			return;
		default:
			std::string msg = "you are at level: " + std::to_string(getLevel());
			_minitel->clearScreen();
			_minitel->println(msg);
	}
	nextLevel();
}

void Maze::print_code() {
	std::lock_guard<std::mutex> lock(_mtx);
	_minitel->clearScreen();
	_minitel->println(_passcode);
}

int Maze::init_minitel() 
{
    Minitel* machine = _minitel;
    byte     reponse;

	std::lock_guard<std::mutex> lock(_mtx);
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
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint8_t> dist;
    std::ostringstream oss;

    oss << std::hex << dist(gen);
	set_pass(oss.str());
	_level = 1;
}

bool Maze::verify_pass(std::string code) {
	std::lock_guard<std::mutex> lock(_mtx);
	if (code != _passcode) {
		return false;
	}
	return true;
}

int Maze::getLevel() const {
	return _level.load();
}

void Maze::nextLevel() {
    _level++;
}
