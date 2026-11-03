#ifndef IMAZE_HPP
#define IMAZE_HPP

#include <string>

class IMaze {
   public:
    virtual ~IMaze() = default;

    virtual void print_code() = 0;
    virtual std::string get_code() = 0;
    virtual bool verify_pass(const std::string& code) = 0;
    virtual void game_over(const std::string& message) = 0;
    virtual void init() = 0;
    virtual void play_game() = 0;
};

#endif  // IMAZE_HPP
