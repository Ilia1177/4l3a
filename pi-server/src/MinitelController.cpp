#include "MinitelController.hpp"
#include "Logger.hpp"
#include <sstream>
#include <iostream>
#include <cstdint>

MinitelController::MinitelController(SessionManager& session, Maze& maze)
    : session_(session), maze_(maze) {}

std::string tokenFromRequest(const httplib::Request& req) 
{
    std::string tok = req.get_header_value("X-Session-Token");
    log_line("Token extract: " + tok + "\n");
    return tok;
}

void MinitelController::registerRoutes(httplib::Server& svr)
{
    svr.Post("/api/minitel/validSession", [this](const httplib::Request& req, httplib::Response& res) {
    	validSession(req, res);
    });
    
    // Join - claim a session
    svr.Post("/api/minitel/join", [this](const httplib::Request& req, httplib::Response& res) {
		join(req, res);
    });

    // Leave - release session
    svr.Post("/api/minitel/leave", [this](const httplib::Request& req, httplib::Response& res) {
		leave(req, res);
    });

    // Print code - validate session and print
    svr.Get("/api/minitel/printcode", [this](const httplib::Request& req, httplib::Response& res) {
		printcode(req, res);
    });

    // Hazardous lab - submit passcode
    svr.Post("/api/minitel/hazardousLab", [this](const httplib::Request& req, httplib::Response& res) {
		submitCode(req, res);
    });
}

void MinitelController::validSession(const httplib::Request& req, httplib::Response& res)
{
    if (!session_.validate(tokenFromRequest(req))) {
        res.status = 401;
        res.set_content("{\"status\": \"token non valide\"}", "application/json");
        return;
    }
    res.set_content("{\"status\": \"token valide\"}", "application/json");
}

void MinitelController::join(const httplib::Request& req, httplib::Response& res)
{
    auto token = session_.tryJoin();
    if (!token) {
        res.status = 409; // Conflict — someone's already playing
        log_line("Session is running");
        res.set_content("{\"error\":\"session busy\"}", "application/json");
        return;
    }
    maze_.init();
    log_line("Minitel init, session joined");
    std::ostringstream json;
    json << "{\"status\":\"joined\",\"token\":\"" << *token << "\"}";
    res.set_content(json.str(), "application/json");
}

void MinitelController::leave(const httplib::Request& req, httplib::Response& res)
{
    session_.release(tokenFromRequest(req));
    res.set_content("{\"status\":\"left\"}", "application/json");
}

void MinitelController::printcode(const httplib::Request& req, httplib::Response& res)
{
    if (!session_.validate(tokenFromRequest(req))) {
        log_line("Not your session...");
        res.status = 403;
        res.set_content("{\"error\":\"not your session\"}", "application/json");
        return;
    }
    maze_.print_code();
    log_line("Code printed on minitel device: " + maze_.get_code());
    res.set_content("{\"status\":\"printed\"}", "application/json");
}

void MinitelController::submitCode(const httplib::Request& req, httplib::Response& res)
{
    if (!session_.validate(tokenFromRequest(req))) {
        log_line("Not your session...");
        res.status = 403;
        res.set_content("{\"error\":\"not your session\"}", "application/json");
        return;
    }
    if (!req.has_param("code")) {
        log_line("no code provided");
        res.status = 400;
        res.set_content("{\"error\":\"missing code\"}", "application/json");
        return;
    }
    std::string code = req.get_param_value("code");
    std::ostringstream oss;
    oss << "[" << timestamp_now() << "] passcode submitted: " << code;
    log_line(oss.str());
    {
        if(!maze_.verify_pass(code)) {
            maze_.game_over("Wrong password...");
            maze_.init();
            log_line("Wrong passcode submitted");
            res.status = 401; // Unauthorized
            res.set_content("{\"error\":\"wrong passcode\"}", "application/json");
            return;
        }
        log_line("Enter hazardous Maze !");
        maze_.game_start();
    }
    res.set_content("{\"status\":\"passcode is correct\"}", "application/json");
}
