#ifndef MINITEL_CONTROLLER_HPP
#define MINITEL_CONTROLLER_HPP

#include "httplib.h"
#include "Maze.hpp"
#include "SessionManager.hpp"
#include "Logger.hpp"
#include <string>

class MinitelController {
public:
    MinitelController(SessionManager& session, Maze& maze);
    
	void registerRoutes(httplib::Server& svr);
    void validSession(const httplib::Request& req, httplib::Response& res);
    void join(const httplib::Request& req, httplib::Response& res);
    void leave(const httplib::Request& req, httplib::Response& res);
    void printcode(const httplib::Request& req, httplib::Response& res);
    void submitCode(const httplib::Request& req, httplib::Response& res);

private:
    SessionManager& session_;
    Maze& maze_;
};

#endif // MINITEL_CONTROLLER_HPP
