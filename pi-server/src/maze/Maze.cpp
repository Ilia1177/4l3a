#include "Maze.hpp"
#include <iomanip>
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
	time << std::setfill('0') << std::setw(3) << left;
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

void Maze::wakeup() {
	_minitel->printChar('\0');
}

std::string get_random_word();

void Maze::set_pass()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;

    oss << std::hex << dist(gen);
	// _passcode = oss.str();
	_passcode = get_random_word();
}

std::string Maze::get_code()
{
	std::lock_guard<std::mutex> lock(_mtx);
	return _passcode;
}

void machine_print_line_centered(Minitel* minitel, std::string line);

void Maze::game_over(std::string msg)
{
	std::lock_guard<std::mutex> lock(_mtx);
	ascii_noise(_minitel, 150);
	_minitel->moveCursorXY(10, 10);
	machine_print_line_centered(_minitel, msg);
	machine_print_line_centered(_minitel, "YOU LOSE !");
	_level = 0;
	std::this_thread::sleep_for(std::chrono::seconds(10));
}

void Maze::level_1() 
{
	ascii_noise(_minitel, 50);
	_minitel->println("What do you know about randomness ?");
}

void Maze::level_2() 
{
	ascii_noise(_minitel, 50);
}

void Maze::play_game() {
	std::lock_guard<std::mutex> lock(_mtx);
	switch(_level) {
		case 1:
			level_1();
			break;
		case 2:
			level_2();
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
	ascii_noise(_minitel, 50);
	_minitel->moveCursorXY(10, 10);
	machine_print_line_centered(_minitel, "You have asked for a pass code: ");
	_minitel->moveCursorRight(10);
	_minitel->attributs(INVERSION_FOND);
	_minitel->attributs(CLIGNOTEMENT);
	_minitel->println(_passcode);
	_minitel->attributs(FIXE);
	_minitel->attributs(FOND_NORMAL);
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
	std::lock_guard<std::mutex> lock(_mtx);
	set_pass();
	_minitel->clearScreen();
	_minitel->moveCursorXY(5, 5);
	machine_print_line_centered(_minitel, "Initialisation...");
	ascii_noise(_minitel, 20);
	_minitel->moveCursorXY(10, 10);
	machine_print_line_centered(_minitel, "Welcome to HAZARDOUS ÉDITORIAL LAB");
	machine_print_line_centered(_minitel, "Get your passcode at: ");
	_minitel->attributs(INVERSION_FOND);
	_minitel->attributs(CLIGNOTEMENT);
	machine_print_line_centered(_minitel, "4l3a.duckdns.org");
	_minitel->attributs(FIXE);
	_minitel->attributs(FOND_NORMAL);
	_level = 0;
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
