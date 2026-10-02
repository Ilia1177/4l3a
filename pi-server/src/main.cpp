#include "httplib.h"
#include "Maze.hpp"
#include "SessionManager.hpp"

#include <csignal>
#include <thread>
#include <mutex>
#include <atomic>

extern void registerRoutes(httplib::Server& srv, Maze& maze);

// Global state shared with routes
SessionManager g_session;
std::mutex g_log_mutex;
std::atomic<uint64_t> g_visit_count{0};

volatile sig_atomic_t g_signal = 0;

void handleSignal(int signal) {
    g_signal = signal;
}

int main() {
    httplib::Server svr;
    HardwareSerial serial;
    Minitel* minitel;
    Maze* maze;

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
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response&) {
        uint64_t count = g_visit_count.fetch_add(1, std::memory_order_relaxed) + 1;
        std::ostringstream oss;
        oss << "[" << __DATE__ << " " << __TIME__ << "] visit #" << count
            << " from " << req.remote_addr
            << " -> " << req.path;
        std::lock_guard<std::mutex> lock(g_log_mutex);
        std::cout << oss.str() << std::endl;
        std::ofstream ofs("visits.log", std::ios::app);
        if (ofs) ofs << oss.str() << "\n";
        return httplib::Server::HandlerResponse::Unhandled;
    });

    registerRoutes(svr, *maze);

    const std::string host = "127.0.0.1";
    const int port = 8080;

    std::cout << "Listening on http://" << host << ":" << port << " (behind Caddy)\n";
    svr.listen(host, port);
    watcher.join();
    delete maze;
    delete minitel;
    return 0;
}