#include "httplib.h"
#include "SessionManager.hpp"
#include "Maze.hpp"

#include <sstream>
#include <iostream>
#include <mutex>
#include <cstdint>

extern std::mutex g_log_mutex;
static std::mutex g_minitel_mutex;
extern SessionManager g_session;
extern std::atomic<uint64_t> g_visit_count;

std::string tokenFromRequest(const httplib::Request& req) {
    return req.get_header_value("X-Session-Token");
}

std::string timestamp_now() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void log_line(const std::string& line) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cout << line << std::endl;

    std::ofstream ofs("visits.log", std::ios::app);
    if (ofs) ofs << line << "\n";
}

void registerRoutes(httplib::Server& srv, Maze& maze) {
    // Join - claim a session
    srv.Post("/api/minitel/join", [&maze](const httplib::Request&, httplib::Response& res) {
        auto token = g_session.tryJoin();
        if (!token) {
            res.status = 409; // Conflict — someone's already playing
            res.set_content("{\"error\":\"session busy\"}", "application/json");
            return;
        }
        maze.init();
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
            res.status = 403;
            res.set_content("{\"error\":\"not your session\"}", "application/json");
            return;
        }

		{
			std::lock_guard<std::mutex> lock(g_minitel_mutex);
			maze.init();
			maze.print_code();
		}

        res.set_content("{\"status\":\"printed\"}", "application/json");
    });

    // Hazardous lab - submit passcode
    srv.Post("/api/minitel/hazardousLab", [&maze](const httplib::Request& req, httplib::Response& res) {
        if (!g_session.validate(tokenFromRequest(req))) {
            res.status = 403;
            res.set_content("{\"error\":\"not your session\"}", "application/json");
            return;
        }
        if (!req.has_param("code")) {
            res.status = 400;
            res.set_content("{\"error\":\"missing code\"}", "application/json");
            return;
        }
        std::string code = req.get_param_value("code");
        std::ostringstream oss;
        oss << "[" << timestamp_now() << "] passcode submitted: " << code;
        log_line(oss.str());
		{
			std::lock_guard<std::mutex> lock(g_minitel_mutex);
			if(!maze.verify_pass(code)) {
				res.status = 402; // Rejection code ???
        		res.set_content("{\"status\":\"wrong pass\"}", "application/json");
				return;
			}
			maze.enter();
		}
        res.set_content("{\"status\":\"sent\"}", "application/json");
    });

    // Status endpoint with visit counter
    srv.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::ostringstream json;
        json << "{\"status\":\"ok\",\"visits\":" << g_visit_count.load() << "}";
        res.set_content(json.str(), "application/json");
    });
}
