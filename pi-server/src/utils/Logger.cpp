#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

static std::mutex g_log_mutex;

std::string timestamp_now() 
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void log_line(const std::string& line)
{
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cout << line << std::endl;

    std::ofstream ofs("visits.log", std::ios::app);
    if (ofs) ofs << line << "\n";
}
