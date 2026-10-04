#include "httplib.h"
#include "Maze.hpp"
#include "SessionManager.hpp"
#include "Logger.hpp"

#include <csignal>
#include <thread>
#include <atomic>

#define VERSION "1.1";

extern void registerRoutes(httplib::Server& srv, Maze& maze);

// Global state shared with routes
SessionManager g_session;
std::atomic<uint64_t> g_visit_count{0};
volatile sig_atomic_t g_signal = 0;

void handleSignal(int signal) {
    g_signal = signal;
}

int main() 
{
	std::cout << "START 4l3A SERVER v" << VERSION;
	std::cout << std::endl;
    httplib::Server svr;
    Minitel* minitel = nullptr;
    Maze* maze = nullptr;
    HardwareSerial serial;
	int status = 0;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    serial.openPort("/dev/ttyUSB0");
    minitel = new Minitel(serial);

	int attempt = 0;
	int max_attempt = 10;
	while(!maze && !g_signal && attempt < max_attempt) {
		try {
			maze = new Maze(minitel);
			std::cout << "Minitel initialisé.\n";
		} catch (std::runtime_error &e) {
			std::cerr << "Error" << e.what() << "\n";
			maze = nullptr;
			attempt--;
		}
	}
	if (attempt < 0) {
		std::cerr << "Error initialisation minitel\n";
		delete minitel;
		return 1;
	}

    // Serve everything in ./public as static files (index.html, css, js...)
    auto ret = svr.set_mount_point("/", "./public");
    if (!ret) {
        std::cerr << "Couldn't mount ./public — does the folder exist next to the binary?\n";
		delete minitel;
        return 1;
    }

    // Watcher thread: polls g_signal, stops the server once it's set
    std::thread watcher([&svr, &status]() {
        while (!g_signal) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
		status = g_signal;
        log_line("\nSignal received, shutting down...\n");
        svr.stop();
    });

	std::thread timeTicker([maze]() {
		while (!g_signal) {
			{
				maze->update_play_time(g_session);
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});

    // Hook that fires on every single request, before it's handled.
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
        uint64_t count = g_visit_count.fetch_add(1, std::memory_order_relaxed) + 1;
        // std::ostringstream oss;
		//       oss << "[" << __DATE__ << " " << __TIME__ << "] visit #" << count
		//           << " from " << req.remote_addr
		//           << " -> " << req.path;
		// log_line(oss.str());
        return httplib::Server::HandlerResponse::Unhandled;
    });

    registerRoutes(svr, *maze);

    const std::string host = "127.0.0.1";
    const int port = 8080;

    std::cout << "Listening on http://" << host << ":" << port << " (behind Caddy)\n";
    svr.listen(host, port);
    watcher.join();
	timeTicker.join();
	std::cout << "Exit server with status: " << status << std::endl;
    delete maze;
    // delete minitel;
    return status;
}
