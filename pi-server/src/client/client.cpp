#include "Client.hpp"

Client::Client(): minitel(nullptr) {}
Client::Client(int fd): minitel(nullptr) {
	serial.setFd(fd);
}

Client::Client(const char* device) 
{
	if (!this->serial.openPort(device))
		throw std::runtime_error(strerror(errno));
	this->minitel = new Minitel(this->serial);
}

std::string Client::current_page_str()
{
	switch(currentPage) {
		case MAIN_PAGE:
			return "main page";
		case RISO:
			return "game one";
		default:
			return "no know state";
	}
}

Client::Client(HardwareSerial& s): serial(s) 
{
	this->minitel = new Minitel(this->serial);
}

void Client::init(HardwareSerial& s) {
	this->minitel = new Minitel(s);
	serial = s;
}

// bool Client::displayPng(const std::string& path, int threshold) {
// 	if (!minitel) return false;
// 	return minitel->displayPng(path, threshold);
// }
//
// bool Client::displayPng(const std::string& path, int maxCellsW, int maxCellsH, int threshold) {
// 	if (!minitel) return false;
// 	return minitel->displayPng(path, maxCellsW, maxCellsH, threshold);
// }

// bool Client::displayPngCentered(const std::string& path, int threshold) {
// 	if (!minitel) return false;
// 	return minitel->displayPng(path, threshold);
// }

Client::~Client() {
	if (minitel) {
		minitel->newScreen();
		minitel->println("Deconnexion.");
		delete minitel;
	}
	if (serial.getFd() > 0) {
		tcdrain(serial.getFd());
		::close(serial.getFd());
	}
}
