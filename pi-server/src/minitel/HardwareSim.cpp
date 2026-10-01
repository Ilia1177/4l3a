#include "HardwareSim.h"
#include <stdexcept>

bool HardwareSerial::openPort(const char* device)
{
    _fd = ::open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return _fd >= 0;
}

void HardwareSerial::begin(unsigned long baud, int config)
{
    (void)config;

    termios tty{};

    if (tcgetattr(_fd, &tty) < 0) {
        perror("1. tcgetattr");
        return;
    }

    cfmakeraw(&tty);

    speed_t s = baudToSpeed(baud);
	if (cfsetispeed(&tty, s) < 0) {
		perror("cfsetispeed");
		return;
	}

	if (cfsetospeed(&tty, s) < 0) {
		perror("cfsetospeed");
		return;
	}

	tty.c_cflag &= ~(CSIZE | PARODD | CSTOPB);
	tty.c_cflag |= CS7 | PARENB;
	tty.c_cflag |= CLOCAL | CREAD;

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(_fd, TCSANOW, &tty) < 0) {
        perror("2. tcsetattr");
        return;
    }

    // Give the serial adapter a DTR transition
    int modem = TIOCM_DTR;

    if (ioctl(_fd, TIOCMBIC, &modem) < 0)
        perror("clear DTR");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    if (ioctl(_fd, TIOCMBIS, &modem) < 0)
        perror("set DTR");

    // Small delay for the Minitel interface to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Optional: make sure no old bytes are waiting
    tcflush(_fd, TCIOFLUSH);
    // Optional: make fd non-blocking
    int flags = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, flags | O_NONBLOCK);

	uint8_t sync[16] = {0};
	::write(_fd, sync, sizeof(sync));
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	tcflush(_fd, TCIFLUSH);
}

void HardwareSerial::end() {}

size_t HardwareSerial::write(uint8_t b)
{
	int r = ::write(_fd, &b, 1);
    if(r < 0) {
		std::string er =  "Error write: " + std::string(strerror(errno));
		throw std::runtime_error(er);
	}
	return r;
}

int HardwareSerial::available()
{
    int n = 0;
    if (ioctl(_fd, FIONREAD, &n) < 0) {
		std::string er =  "Error ioctl: " + std::string(strerror(errno));
		throw std::runtime_error(er);
    }
    return n;
}

int HardwareSerial::read()
{
    uint8_t b;

	if (::read(_fd, &b, 1) < 0) {
		std::string er = "Error read: " + std::string(strerror(errno));
		throw std::runtime_error(er);
	}
	return b;
}

HardwareSerial::operator bool() const 
{ 
	return _fd >= 0; 
}
void HardwareSerial::setFd(int fd) 
{
	_fd = fd; 
}
int HardwareSerial::getFd() 
{
	return _fd; 
}

speed_t HardwareSerial::baudToSpeed(unsigned long baud)
{
    switch (baud) {
    case 300:
        return B300;
    case 1200:
        return B1200;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    default:
        return B1200;
    }
}
