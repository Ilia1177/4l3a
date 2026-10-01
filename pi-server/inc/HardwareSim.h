#ifndef HARDWARESIM_H
#define HARDWARESIM_H

#include <cstdint>
#include <string>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef bool boolean;

#define SERIAL_7E1 0

#define highByte(w) ((byte)((w) >> 8))
#define lowByte(w)  ((byte)((w) & 0xFF))
#define bitRead(v,b) (((v) >> (b)) & 0x01)
#define bitWrite(value, bit, bitvalue) \
    ((bitvalue) ? ((value) |= (1UL << (bit))) : ((value) &= ~(1UL << (bit))))

extern volatile sig_atomic_t g_signal;

inline unsigned long millis() {
    using namespace std::chrono;
    static auto t0 = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - t0).count();
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class String : public std::string {
public:
    using std::string::string;
    String(const std::string& s) : std::string(s) {}
    char charAt(size_t i) const { return (*this)[i]; }
    int lastIndexOf(char c) const {
        auto pos = find_last_of(c);
        return pos == npos ? -1 : (int)pos;
    }
};
#include <cstring>   // strerror
#include <cerrno>
#include <iostream>

class HardwareSerial {
	public:
		bool openPort(const char* device);
		void begin(unsigned long baud, int /*config*/ = 0);
		void end();
		size_t write(uint8_t b);
		int available();
		int read();
		explicit operator bool() const;

		int getFd();
		void setFd(int f);

	private:
		int _fd = -1;
		speed_t baudToSpeed(unsigned long baud);
};

#endif
