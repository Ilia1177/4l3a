#include "httplib.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

#include "Minitel1B_Hard.h"
#include "HardwareSim.h"
#include "Client.hpp"
#include <csignal>



volatile sig_atomic_t g_signal = 0;
// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string timestamp_now() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// A tiny mutex-guarded log file, since this can be called from multiple
// worker threads (httplib is multi-threaded by default).
static std::mutex g_log_mutex;
static void log_line(const std::string& line) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cout << line << std::endl;

    std::ofstream ofs("visits.log", std::ios::app);
    if (ofs) ofs << line << "\n";
}

static std::atomic<uint64_t> g_visit_count{0};

// ---------------------------------------------------------------------------
// THIS is where you plug in whatever should happen when a client hits the
// page: log it, flip a GPIO pin, push a webhook, write to a database, etc.
// Keep it fast (or dispatch to a background thread) since it runs inline
// with the request.
// ---------------------------------------------------------------------------
static void onClientVisit(const httplib::Request& req) {
    uint64_t count = ++g_visit_count;

    std::ostringstream oss;


	std::string client_ip = req.remote_addr; // fallback
    if (req.has_header("X-Forwarded-For")) {
        client_ip = req.get_header_value("X-Forwarded-For");
    }
    oss << "[" << timestamp_now() << "] visit #" << count
        << " from " << req.remote_addr
        << " -> " << req.path;
    log_line(oss.str());
	// cli->minitel->println("Hello");

    // Examples of what you could do here instead / in addition:
    //
    //   - Toggle a GPIO pin (e.g. with pigpio or libgpiod):
    //       gpioWrite(17, 1);
    //
    //   - Fire a webhook / notification:
    //       httplib::Client cli("https://hooks.example.com");
    //       cli.Post("/notify", "someone visited", "text/plain");
    //     (do this on a detached thread so it never blocks the response)
    //
    //   - Write structured data to a file/db for later analysis.
}

void handleSignal(int signal) {
    g_signal = signal;
}

int main() {

	// Client *scr;
	//
	// scr = new Client("/dev/ttyUSB0");
	httplib::Server svr;

	HardwareSerial serial;
	serial.openPort("/dev/ttyUSB0");
	Minitel *minitel = new Minitel(serial);

	std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);



    // Serve everything in ./public as static files (index.html, css, js...)
    auto ret = svr.set_mount_point("/", "./public");
    if (!ret) {
        std::cerr << "Couldn't mount ./public — does the folder exist next to the binary?\n";
        return 1;
    }

    // Hook that fires on every single request, before it's handled.
    // This is the simplest way to guarantee onClientVisit() runs no matter
    // which file/route was requested.
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
        onClientVisit(req);
        return httplib::Server::HandlerResponse::Unhandled; // let normal routing continue
    });

	svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
		onClientVisit(req);
		return httplib::Server::HandlerResponse::Unhandled;
	});
		
	svr.Get("/api/minitel", [minitel](const httplib::Request&, httplib::Response& res) {
    minitel->println("Hello");
    res.set_content("{\"status\":\"printed\"}", "application/json");
	});

    // Example JSON API route, so you can see the visit counter working.
    svr.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::ostringstream json;
        json << "{\"status\":\"ok\",\"visits\":" << g_visit_count.load() << "}";
        res.set_content(json.str(), "application/json");
    });

    // Bind to localhost only — Caddy is the public-facing TLS proxy.
    const std::string host = "127.0.0.1";
    const int port = 8080;

    std::cout << "Listening on http://" << host << ":" << port << " (behind Caddy)\n";
    svr.listen(host, port);

    return 0;
}
