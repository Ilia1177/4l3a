#ifndef MINITEL_CONTROLLER_HPP
#define MINITEL_CONTROLLER_HPP

#include <string>

#include "IMaze.hpp"
#include "Logger.hpp"
#include "SessionManager.hpp"
#include "httplib.h"

class MinitelController {
   public:
    MinitelController(SessionManager& session, IMaze& maze);

    void registerRoutes(httplib::Server& svr);
    void validSession(const httplib::Request& req, httplib::Response& res);
    void join(const httplib::Request& req, httplib::Response& res);
    void leave(const httplib::Request& req, httplib::Response& res);
    void printcode(const httplib::Request& req, httplib::Response& res);
    void submitCode(const httplib::Request& req, httplib::Response& res);

   private:
    SessionManager& session_;
    IMaze& maze_;
};

#endif  // MINITEL_CONTROLLER_HPP
