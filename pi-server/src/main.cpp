#include "httplib.h"
#include "Maze.hpp"
#include "SessionManager.hpp"
#include "Logger.hpp"

#include <csignal>
#include <thread>
#include <atomic>
#include "MinitelController.hpp"
#include "routes.hpp"
#define VERSION "1.1";

// Global state shared with routes
SessionManager g_session;
std::atomic<uint64_t> g_visit_count{0};
volatile sig_atomic_t g_signal = 0;

void handleSignal(int signal) {
    g_signal = signal;
}

int main() 
{
	std::cout << "START HaZ4rDou$ SERVER v" << VERSION;
	std::cout << std::endl;
    httplib::Server svr;
    Maze* maze = nullptr;
	int status = 0;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

	maze = new Maze();
	int attempt = 1;
	while(!g_signal && attempt < 10) {
		try {
			maze->init_minitel();
			std::cout << "Minitel initialisé a l'essai " << attempt << "\n";
			break;
		} catch (std::runtime_error &e) {
			std::cerr << "Error" << e.what() << "attempt: " << attempt << "\n";
			attempt++;
		}
	}
	MinitelController controller(g_session, *maze);

    // Serve everything in ./public as static files (index.html, css, js...)
    auto ret = svr.set_mount_point("/", "./public");
    if (!ret) {
        std::cerr << "Couldn't mount ./public — does the folder exist next to the binary?\n";
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
			if(g_session.is_valid() && g_session.get_time_left() >= 0) {
				maze->update_play_time(g_session);
			} else if (g_session.is_valid()) {
				maze->game_over("Time OUT !");
				g_session.invalidate();
				std::this_thread::sleep_for(std::chrono::seconds(10));
				maze->init();
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});


    registerRoutes(svr, *maze, controller);

    const std::string host = "127.0.0.1";
    const int port = 8080;

    std::cout << "Listening on http://" << host << ":" << port << " (behind Caddy)\n";
    svr.listen(host, port);
    watcher.join();
	timeTicker.join();
	std::cout << "Exit server with status: " << status << std::endl;
    delete maze;
    return status;
}
