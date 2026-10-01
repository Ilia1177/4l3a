#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Minitel1B_Hard.h"

enum { 
	MAIN_PAGE = 1, 
	RISO = 2,
	CONNINFO = 3,
	SYSTEM = 4
};

class Client
{
  public:
	Client(int fd);
    Client(const char* device);
    Client(HardwareSerial& serial);
    Client(void);
    ~Client();
	void killShell(bool fast = false);
    std::string    current_page_str();
    void           init(HardwareSerial& s);
    // PNG -> G1 mosaic (centered, B&W threshold). Delegates to Minitel::displayPng
    // bool           displayPng(const std::string& path, int threshold = 128);
    // bool           displayPng(const std::string& path, int maxCellsW, int maxCellsH, int threshold = 128);
    // bool           displayPngCentered(const std::string& path, int threshold = 128);
    int            index;
    HardwareSerial serial;
    Minitel*       minitel;
    int            currentPage;
  private:
};

#endif
