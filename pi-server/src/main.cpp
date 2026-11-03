#include <atomic>
#include <csignal>
#include <thread>

#include "Logger.hpp"
#include "Maze.hpp"
#include "MinitelController.hpp"
#include "SessionManager.hpp"
#include "httplib.h"
#include "routes.hpp"

#define VERSION "1.1"

SessionManager g_session;
std::atomic<uint64_t> g_visit_count{0};
volatile sig_atomic_t g_signal = 0;

void handleSignal(int signal) { g_signal = signal; }

namespace {

int runServer() {
    std::cout << "START HaZ4rDou$ SERVER v" << VERSION << '\n';
    httplib::Server svr;
    Maze* maze = nullptr;
    int status = 0;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    maze = new Maze();
    int attempt = 1;
    while (!g_signal && attempt < 10) {
        try {
            maze->init_minitel();
            std::cout << "Minitel initialisé a l'essai " << attempt << "\n";
            break;
        } catch (std::runtime_error& error) {
            std::cerr << "Error" << error.what() << " on attempt " << attempt << "\n";
            attempt++;
        }
    }
    MinitelController controller(g_session, *maze);

    auto ret = svr.set_mount_point("/", "./public");
    if (!ret) {
        std::cerr << "Couldn't mount ./public — does the folder exist next to the binary?\n";
        delete maze;
        return 1;
    }

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
            if (g_session.is_valid()) {
                maze->update_play_time(g_session);
                if (g_session.get_time_left() <= 0) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    maze->game_over("Time OUT !");
                    g_session.invalidate();
                    maze->init();
                }
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
    std::cout << "Exit server with status: " << status << '\n';
    delete maze;
    return status;
}

}  // namespace

int main() {
    try {
        return runServer();
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Fatal unknown error\n";
        return 1;
    }
}
