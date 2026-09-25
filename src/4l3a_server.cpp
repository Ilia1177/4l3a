// 4l3a_server.cpp
// Minimal HTTP server: listens on a port, accepts connections,
// reads the HTTP request (ignored/logged), and replies with an HTML page.
//
// Build:   make
// Run:     ./4l3a_server 30777
// Test:    open http://<pi-ip>:30777 in a browser, or:
//          curl http://<pi-ip>:30777

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// The HTML page we send back for every request.
// Keeping it as one string literal for now; later you could load it from a file.
static const std::string HTML_BODY =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head><title>4l3a server</title></head>\n"
    "<body>\n"
    "<h1>Hello from the Raspberry Pi!</h1>\n"
    "<p>This page is served by a C++ TCP server.</p>\n"
    "</body>\n"
    "</html>\n";

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Listening on port %d...\n", port);

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        // Read (and discard) the incoming HTTP request.
        // We don't parse it yet, just drain it so the client isn't left hanging.
        char buffer[4096];
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Request from %s:\n%s\n", client_ip, buffer);
        }

        // Build the HTTP response: status line, headers, blank line, then body.
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + std::to_string(HTML_BODY.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            HTML_BODY;

        write(client_fd, response.c_str(), response.size());
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
