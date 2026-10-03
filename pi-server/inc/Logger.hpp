#ifndef LOGGER_HPP
# define LOGGER_HPP
# include <iostream>
# include <mutex>

std::string timestamp_now();
void log_line(const std::string& s);
# endif
