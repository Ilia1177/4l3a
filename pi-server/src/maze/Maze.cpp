#include "Maze.hpp"
#include <sstream>
#include <random>

#define MINITEL_PATH "/dev/ttyUSB0"

Maze::Maze(): _time_left(0), _level(0), _minitel(nullptr), _passcode("") 
{
	if(init_minitel() < 0) {
		throw std::runtime_error("Minitel init fail");
	}
}

Maze::~Maze() { if (_minitel) delete _minitel; }

void Maze::update_play_time(SessionManager& session) {
	std::lock_guard<std::mutex> lock(_mtx);
	int left = session.get_time_left();
	std::ostringstream time;
	time << std::setfill('0') << std::setw(2) << left;
	_minitel->println00(time.str());
}

void ascii_noise(Minitel* minitel, int amount) {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> distChar(33, 126); // printable ASCII range, adjust as needed
    std::uniform_int_distribution<int> distX(1, 40);
    std::uniform_int_distribution<int> distY(1, 25);

    while (amount > 0) {
        int x = distX(gen);
        int y = distY(gen);
        char caractere = static_cast<char>(distChar(gen));

        minitel->moveCursorXY(x, y);
        minitel->printChar(caractere);
        amount--;
    }
}

void Maze::set_pass()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;

	std::lock_guard<std::mutex> lock(_mtx);
    oss << std::hex << dist(gen);
	_passcode = oss.str();
}

std::string Maze::get_code() {
	return _passcode;
}

void print_center_x(Minitel* minitel, std::string msg, int x = 40) 
{
	const int msglen = msg.length();

	int start = minitel->getCursorX();
	const int screenlen = x - start;
	if (msglen > screenlen) {
		minitel->println(msg);
		return;
	}
	int space = (screenlen - msglen) / 2;
	if (space > 0) {
		minitel->printChar(' ');
		minitel->repeat(space - 1);
	}
	minitel->println(msg);
}

void Maze::game_over(std::string msg)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_minitel->clearScreen();
	ascii_noise(_minitel, 150);
	_minitel->moveCursorXY(7, 12);
	_minitel->println(msg);
	_minitel->println("YOU LOSE");
	_level = 1;
}

void Maze::game_start() {
	std::lock_guard<std::mutex> lock(_mtx);
	switch(_level) {
		case 1:
			ascii_noise(_minitel, 200);
			break;
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
	std::lock_guard<std::mutex> lock(_mtx);
    byte     reponse;

    if (!_serial.openPort(MINITEL_PATH)) {
		throw std::runtime_error("serial port: not opened");
	}
	_minitel = new Minitel(_serial);
    _minitel->newScreen();
    reponse = _minitel->echo(false);
	if (reponse == 0x44) {
		_minitel->modeVideotex();  // Mode Mixte => Mode Vidéotex 40 colonnes
		_minitel->noCursor();
		_minitel->smallMode();
  		_minitel->extendedKeyboard();  // Clavier étendu
		return 0;
	}
	return -1;
}

void Maze::init() 
{
	set_pass();
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
