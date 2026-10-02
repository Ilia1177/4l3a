#include "httplib.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

#include "Maze.hpp"

#include <csignal>

#include <random>
#include <optional>

class SessionManager {
public:
    // Try to claim the session. Returns a token on success, nullopt if busy.
    std::optional<std::string> tryJoin() {
        std::lock_guard<std::mutex> lock(mtx_);
        reapIfExpired();

        if (!token_.empty()) {
            return std::nullopt; // someone else is already playing
        }

        token_ = generateToken();
        lastActivity_ = std::chrono::steady_clock::now();
        return token_;
    }

    // Check a request's token is the current owner; refreshes activity if valid.
    bool validate(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx_);
        reapIfExpired();

        if (token.empty() || token != token_) return false;
        lastActivity_ = std::chrono::steady_clock::now();
        return true;
    }

    // Explicit release (e.g. user clicks "leave" or closes the tab cleanly).
    void release(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (token == token_) {
            token_.clear();
        }
    }

private:
    std::mutex mtx_;
    std::string token_;
    std::chrono::steady_clock::time_point lastActivity_;
    static constexpr int kTimeoutSeconds = 60;

    void reapIfExpired() {
        if (token_.empty()) return;
        auto idle = std::chrono::steady_clock::now() - lastActivity_;
        if (idle > std::chrono::seconds(kTimeoutSeconds)) {
            token_.clear(); // previous player went idle/disappeared
        }
    }

    std::string generateToken() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dist;
        std::ostringstream oss;
        oss << std::hex << dist(gen);
        return oss.str();
    }
};

static SessionManager g_session;

volatile sig_atomic_t g_signal = 0;
// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::string tokenFromRequest(const httplib::Request& req) {
    return req.get_header_value("X-Session-Token");
}
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
}

void handleSignal(int signal) {
    g_signal = signal;
}

void routes(httplib::Server& srv) {

}
int main() 
{
	httplib::Server svr;
	HardwareSerial serial;
	Minitel *minitel;
	Maze *maze;

	serial.openPort("/dev/ttyUSB0");
	minitel = new Minitel(serial);
	minitel->clearScreen();

	maze = new Maze(minitel);

	std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);


    // Serve everything in ./public as static files (index.html, css, js...)
    auto ret = svr.set_mount_point("/", "./public");
    if (!ret) {
        std::cerr << "Couldn't mount ./public — does the folder exist next to the binary?\n";
        return 1;
    }

    // Watcher thread: polls g_signal, stops the server once it's set
    std::thread watcher([&svr]() {
        while (!g_signal) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::cout << "\nSignal received, shutting down...\n";
        svr.stop();
    });
    // Hook that fires on every single request, before it's handled.
    // This is the simplest way to guarantee onClientVisit() runs no matter
    // which file/route was requested.
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
        onClientVisit(req);
        return httplib::Server::HandlerResponse::Unhandled; // let normal routing continue
    });

	// svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
	// 	onClientVisit(req);
	// 	return httplib::Server::HandlerResponse::Unhandled;
	// });
		
	svr.Post("/api/minitel/join", [maze](const httplib::Request&, httplib::Response& res) {
		auto token = g_session.tryJoin();
		if (!token) {
			res.status = 409; // Conflict — someone's already playing
			res.set_content("{\"error\":\"session busy\"}", "application/json");
			return;
		}

		maze->init();
		std::ostringstream json;
		json << "{\"status\":\"joined\",\"token\":\"" << *token << "\"}";
		res.set_content(json.str(), "application/json");
	});

	svr.Post("/api/minitel/leave", [](const httplib::Request& req, httplib::Response& res) {
		g_session.release(tokenFromRequest(req));
		res.set_content("{\"status\":\"left\"}", "application/json");
	});
	svr.Get("/api/minitel/printcode", [maze](const httplib::Request& req, httplib::Response& res) {
		if (!g_session.validate(tokenFromRequest(req))) {
			res.status = 403;
			res.set_content("{\"error\":\"not your session\"}", "application/json");
			return;
		}
		res.set_content("{\"status\":\"printed\"}", "application/json");
		maze->init();
	});
	svr.Post("/api/minitel/join", [maze](const httplib::Request&, httplib::Response& res) {
		auto token = g_session.tryJoin();
		if (!token) {
			res.status = 409; // Conflict — someone's already playing
			res.set_content("{\"error\":\"session busy\"}", "application/json");
			return;
		}

		maze->init();
		std::ostringstream json;
		json << "{\"status\":\"joined\",\"token\":\"" << *token << "\"}";
		res.set_content(json.str(), "application/json");
	});

	svr.Post("/api/minitel/leave", [](const httplib::Request& req, httplib::Response& res) {
		g_session.release(tokenFromRequest(req));
		res.set_content("{\"status\":\"left\"}", "application/json");
	});

svr.Get("/api/minitel/printcode", [maze](const httplib::Request& req, httplib::Response& res) {
    if (!g_session.validate(tokenFromRequest(req))) {
        res.status = 403;
        res.set_content("{\"error\":\"not your session\"}", "application/json");
        return;
    }
    res.set_content("{\"status\":\"printed\"}", "application/json");
    maze->init();
});

svr.Post("/api/minitel/hazardousLab", [minitel, maze](const httplib::Request& req, httplib::Response& res) {
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

    maze->verify_pass(code);
    maze->enter();
    res.set_content("{\"status\":\"sent\"}", "application/json");
});
		svr.Post("/api/minitel/hazardousLab", [minitel, maze](const httplib::Request& req, httplib::Response& res) {
			if (!req.has_param("code")) {
				res.status = 400;
				res.set_content("{\"error\":\"missing code\"}", "application/json");
				return;
			}
			std::string code = req.get_param_value("code");
			std::ostringstream oss;
			oss << "[" << timestamp_now() << "] passcode submitted: " << code;
			log_line(oss.str());

			maze->verify_pass(code);
			maze->enter();
			res.set_content("{\"status\":\"sent\"}", "application/json");
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
    watcher.join();
	delete minitel;
    return 0;
}
