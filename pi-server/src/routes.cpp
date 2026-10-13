#include "httplib.h"
#include "SessionManager.hpp"
#include "Maze.hpp"
#include "Logger.hpp"
#include "MinitelController.hpp"

#include <sstream>
#include <iostream>
#include <cstdint>

// static std::mutex g_minitel_mutex;
extern SessionManager g_session;
extern std::atomic<uint64_t> g_visit_count;

// void minitelRoutes(httplib::Server& srv, Maze& maze)
// {
//     MinitelController controller(g_session, maze);
//
//     srv.Post("/api/minitel/validSession", [&controller](const httplib::Request& req, httplib::Response& res) {
//         controller.validSession(req, res);
//     });
//
//     // Join - claim a session
//     srv.Post("/api/minitel/join", [&controller](const httplib::Request& req, httplib::Response& res) {
//         controller.join(req, res);
//     });
//
//     // Leave - release session
//     srv.Post("/api/minitel/leave", [&controller](const httplib::Request& req, httplib::Response& res) {
//         controller.leave(req, res);
//     });
//
//     // Print code - validate session and print
//     srv.Get("/api/minitel/printcode", [&controller](const httplib::Request& req, httplib::Response& res) {
//         controller.printcode(req, res);
//     });
//
//     // Hazardous lab - submit passcode
//     srv.Post("/api/minitel/hazardousLab", [&controller](const httplib::Request& req, httplib::Response& res) {
//         controller.hazardousLab(req, res);
//     });
// }
//
void registerRoutes(httplib::Server& srv, Maze& maze)
{
    // Status endpoint with visit counter
    srv.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::ostringstream json;
        json << "{\"status\":\"ok\",\"visits\":" << g_visit_count.load() << "}";
        res.set_content(json.str(), "application/json");
    });
}
