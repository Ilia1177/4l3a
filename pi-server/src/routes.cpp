#include "routes.hpp"
#include "SessionManager.hpp"

#include <sstream>
#include <cstdint>

extern SessionManager g_session;
extern std::atomic<uint64_t> g_visit_count;

void registerRoutes(httplib::Server& srv, Maze& maze, MinitelController& controller)
{
    // Single pre-routing hook for every request.
    srv.set_pre_routing_handler([&maze](const httplib::Request& req, httplib::Response&) {
        g_visit_count.fetch_add(1, std::memory_order_relaxed);

        const bool isPageLoad = (req.path == "/" || req.path == "/index.html");
        if (isPageLoad && !g_session.is_valid()) {
            maze.init();
        } else if ( isPageLoad ) {
			maze.wakeup()
		}
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Status endpoint with visit counter
    srv.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::ostringstream json;
        json << "{\"status\":\"ok\",\"visits\":" << g_visit_count.load() << "}";
        res.set_content(json.str(), "application/json");
    });

    controller.registerRoutes(srv);
}
