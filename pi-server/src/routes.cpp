#include "httplib.h"
#include "SessionManager.hpp"
#include "Maze.hpp"
#include "Logger.hpp"

#include <sstream>
#include <iostream>
#include <cstdint>

// static std::mutex g_minitel_mutex;
extern SessionManager g_session;
extern std::atomic<uint64_t> g_visit_count;

std::string tokenFromRequest(const httplib::Request& req) 
{
	std::string tok = req.get_header_value("X-Session-Token");
	log_line("Token extract: " + tok + "\n");
    return tok;
}

void minitelRoutes(httplib::Server& srv, Maze& maze)
{
    srv.Post("/api/minitel/validSession", [&maze](const httplib::Request& req, httplib::Response& res) {
			if(!g_session.validate(tokenFromRequest(req))) {
				res.status = 401;
				res.set_content("{\"status\": \"token non valide\"}", "application/json");
				return;
			}
			res.set_content("{\"status\": \"token valide\"}", "application/json");
	});
    // Join - claim a session
    srv.Post("/api/minitel/join", [&maze](const httplib::Request&, httplib::Response& res) {
        auto token = g_session.tryJoin();
        if (!token) {
            res.status = 409; // Conflict — someone's already playing
			log_line("Session is running");
            res.set_content("{\"error\":\"session busy\"}", "application/json");
            return;
        }
		{
			maze.init();
			log_line("Minitel init, session joined");
		}
        std::ostringstream json;
        json << "{\"status\":\"joined\",\"token\":\"" << *token << "\"}";
        res.set_content(json.str(), "application/json");
    });

    // Leave - release session
    srv.Post("/api/minitel/leave", [](const httplib::Request& req, httplib::Response& res) {
        g_session.release(tokenFromRequest(req));
        res.set_content("{\"status\":\"left\"}", "application/json");
    });

    // Print code - validate session and print
    srv.Get("/api/minitel/printcode", [&maze](const httplib::Request& req, httplib::Response& res) {
        if (!g_session.validate(tokenFromRequest(req))) {
			log_line("Not your session...");
            res.status = 403;
            res.set_content("{\"error\":\"not your session\"}", "application/json");
            return;
        }
		maze.print_code();
		log_line("Code printed on minitel device: " + maze.get_code());
        res.set_content("{\"status\":\"printed\"}", "application/json");
    });

    // Hazardous lab - submit passcode
    srv.Post("/api/minitel/hazardousLab", [&maze](const httplib::Request& req, httplib::Response& res) {
        if (!g_session.validate(tokenFromRequest(req))) {
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
			if(!maze.verify_pass(code)) {
				maze.game_over("Wrong password...");
				maze.init();
        		log_line("Wrong passcode submitted");
				res.status = 401; // Unauthorized
        		res.set_content("{\"error\":\"wrong passcode\"}", "application/json");
				return;
			}
        	log_line("Enter hazardous Maze !");
			maze.enter();
		}
        res.set_content("{\"status\":\"passcode is correct\"}", "application/json");
    });

}

void registerRoutes(httplib::Server& srv, Maze& maze)
{
	minitelRoutes(srv, maze);
    // Status endpoint with visit counter
    srv.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::ostringstream json;
        json << "{\"status\":\"ok\",\"visits\":" << g_visit_count.load() << "}";
        res.set_content(json.str(), "application/json");
    });
}
