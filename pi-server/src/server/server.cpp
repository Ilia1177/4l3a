#include "Server.hpp"
#include <csignal>
#include <poll.h>

char appendCodepoint(std::string& input, unsigned long code)
{
    if (code == 0) return 0; // no key

    // Filter out non-printable function/navigation codes if you track them separately
    // (e.g. codes still shaped like 0x1B5B.., or bare 0x13.. function keys)

    if (code < 0x80) {
		char c = static_cast<char>(code);
		input += c;
		return c;
    } else {
        // UTF-8 encode the Unicode code point
        if (code < 0x800) {
            input += static_cast<char>(0xC0 | (code >> 6));
            input += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            input += static_cast<char>(0xE0 | (code >> 12));
            input += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            input += static_cast<char>(0x80 | (code & 0x3F));
        }
    }
	return 0;
}


Server::Server()
{
	signal(SIGPIPE, SIG_IGN);
};

Server::~Server()
{
    for (size_t i = 0; i < _clients.size(); i++) {
        delete _clients[i];
    }
}

int Server::add_listening_stdin() {
    struct pollfd stdinPfd{};
    stdinPfd.fd = STDIN_FILENO;
    stdinPfd.events = POLLIN;
    _pfds.push_back(stdinPfd);
	log("server will be listening stdin on 0", INFO);
	return _pfds.size() - 1;
 }

int Server::add_client(int fd)
{
    Client* new_client;
    pollfd  new_pfd;

	new_client = nullptr;
	log("Adding new client from FD (port 30777)...", INFO);
	new_client = new Client(fd);
 
	if (g_signal)
		return g_signal;
    new_pfd.events = POLLIN;
    new_pfd.revents = 0;
    new_pfd.fd = fd;
    _clients.push_back(new_client);
    _pfds.push_back(new_pfd);
	log("New client added from 30777!", INFO);
    return 0;
}
int Server::add_client(const char* device_path)
{
    Client* new_client;
    pollfd  new_pfd;
	int status;

	new_client = nullptr;
	log("Adding new client...", INFO);
	do {
		log("Trying...", INFO);
		if (new_client)
			delete new_client;
		try {
			new_client = new Client(device_path);
    		new_client->index = _clients.size() + 1;
		} catch (std::exception& e) {
			log(e.what(), ERR);
			return -1;
		}
		status = init_machine(new_client);
		if (status < 0) {
			log("fail machine initialisation !", WARN);
		}
	} while(status < 0 && !g_signal);
	if (g_signal)
		return g_signal;
    new_pfd.events = POLLIN;
    new_pfd.revents = 0;
    new_pfd.fd = new_client->serial.getFd();
    _clients.push_back(new_client);
    _pfds.push_back(new_pfd);
	log("New client added !", INFO);
    return 0;
}

int Server::init_machine(Client* client)
{
    Minitel* machine = client->minitel;
    byte     reponse;

	std::stringstream ss;
    ss << "init machine of client " << client->index;
	log(ss.str(), INFO);
	ss.clear();
    machine->newScreen();
    reponse = machine->echo(false);
	if (reponse == 0x44) {
		client->minitel->smallMode();
  		client->minitel->extendedKeyboard();  // Clavier étendu
		main_page(client);
		return 0;
	}
	ss << "Error: answer: 0x" << std::hex << static_cast<unsigned int>(reponse);
	log(ss.str(), ERR);
	return -1;
}

int Server::handle_client_input(Client* client)
{
	int r = 0;
	log("Handle client input", INFO);
	std::string buff = "";
    if (g_signal || !client) 
		return 0;
	if(!client->minitel) {
		log("read from 30777 client: ", WARN);
		while(client->serial.available()) {
			r++;
			std::string c(1, static_cast<char>(client->serial.read());
			buff += c;
			std::cout << c;
		}
		return r;
	}
    int ret;


	log("Handle MINITEL client input", INFO);
	ret = 0;
    switch (client->currentPage) {
		case MAIN_PAGE:
			ret = main_page_input(client);
			break;
		case RISO:
			ret = risographie_input(client);
			break;
		case CONNINFO:
			ret = connexion_input(client);
			break;
		default:
			break;
    }
    return ret + 1;
}

std::vector<std::string> parse_command(std::string& cmd)
{
	std::istringstream iss(cmd);
	std::string word;
	std::vector<std::string> args;

	while (iss >> word) {
		args.push_back(word);
	}
	return args;
}
// Client* Server::get_client()

// commands are:
// state -> print state of each clients
// send -> send message to client number
int Server::handle_server_command(std::string& cmd)
{
    std::istringstream iss(cmd);
    std::string        word;
	std::vector<std::string> args;

	args = parse_command(cmd);

	if(args.size() > 0 && args[0] == "send") {
		log("SERVER SEND COMMAND", INFO);
	}
    return 0;
}

void Server::log(std::string str, log_level status)
{
	switch(status) {
		case ERR: 
			std::cout << RED;
			break;
		case WARN:
			std::cout << YELLOW;
			break;
		case INFO:
			std::cout << BLUE;
			break;
	}
	std::cout << str << ENDC << std::endl << std::flush;
	// std::cout << ">" << std::flush;
}

//
// // _pfds[0] contiens stdin pour ecouter les commandes du server
// // _pfds s'étends entre 0 (stdin) et _clients.size (dernier client)
#include "poll.h"
#include <sys/socket.h>
#include <netinet/in.h>

int Server::add_listening_port() {

	int port_30777_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (port_30777_fd < 0) {
		perror("socket");
		return 1;
	}
	int opt = 1;
	setsockopt(port_30777_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(30777);

	if (bind(port_30777_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}
	if (::listen(port_30777_fd, 16) < 0) {
		perror("listen");
		return 1;
	}

	// Make it non-blocking so a stalled accept() never freezes poll()
	int flags = fcntl(port_30777_fd, F_GETFL, 0);
	fcntl(port_30777_fd, F_SETFL, flags | O_NONBLOCK);

	pollfd listen_pfd{};
	listen_pfd.fd = port_30777_fd;
	listen_pfd.events = POLLIN;
	_pfds.push_back(listen_pfd); // at index 2
	return _pfds.size() - 1;
}
bool sendAll(int fd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = write(fd, data.data() + sent, data.size() - sent);
        if (n > 0) {
            sent += n;
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // Socket buffer is full right now — for simplicity, just stop here.
            // A more complete implementation would queue the remainder and
            // retry on the next POLLOUT event rather than looping/blocking.
            log("Client socket busy, response incomplete this pass", WARN);
            return false;
        } else if (n < 0 && errno == EINTR) {
            continue; // interrupted, just retry
        } else {
            // n == 0 or a real error (e.g. EPIPE) — client is gone
            return false;
        }
    }
    return true;
}
void serveHtmlPage(int client_fd) {
    static const std::string html_body =
        "<!DOCTYPE html>\n"
        "<html><head><title>4l3a server</title></head>\n"
        "<body><h1>Hello from the Raspberry Pi!</h1></body></html>\n";

    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(html_body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        html_body;

    sendAll(client_fd, response);
}
int Server::listen()
{
	
	log("sever will be listening on port 30777 at index 1", INFO);
    log("start Listening clients", INFO);
    while (!g_signal) {
        int ret = poll(_pfds.data(), _pfds.size(), POLL_TIMEOUT);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        } else if (ret == 0) {
            continue;
        }

	std::vector<size_t> to_remove;
	std::cout << "retour :" << ret << std::endl;
    log("Iterate throught clients", INFO);
	for (size_t i = 0; i < _pfds.size(); i++) {
	    if (i == 0 && _pfds[i].revents & POLLIN) {
			log("Read from STDIN",INFO);
			std::string line;
			std::getline(std::cin, line);
			handle_server_command(line);
	    } else if (i == 1 && _pfds[i].revents & POLLIN) {
			log("New connection FROM PORT 30777", WARN);
			std::cout << "30777 POLLIN add HTTP client " << i << std::endl;
			sockaddr_in client_addr{};
			socklen_t len = sizeof(client_addr);
			int client_fd = accept(_pfds[i].fd, (sockaddr*)&client_addr, &len);
			if (client_fd >= 0) {
				fcntl(client_fd, F_SETFL, O_NONBLOCK);
				add_client(client_fd);
			} else {
				log("ADD Client failed", ERR);
			}
		} else if (_pfds[i].revents & POLLIN) {
			std::cout << "general POLLIN " << i << std::endl;
			Client* cli = _clients[i-2];
			if (!cli->serial.available()) {
				log("Client disconnect", ERR);
				to_remove.push_back(i);
				continue;
			}
			handle_client_input(cli);
	    } else if (_pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
			log("Poll error on serial port", ERR);
			break;
	    }
	    _pfds[i].revents = 0;
	}
	// Erase back-to-front so earlier indices in the list stay valid
	for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
		log("Remove client", INFO);
		size_t i = *it;
		close(_pfds[i].fd);
		_pfds.erase(_pfds.begin() + i);
		_clients.erase(_clients.begin() + (i - 2)); // keep this offset consistent with wherever clients actually start
	}
    }
    return 0;
}

int Server::handle_client_command(Client* client, std::string& command)
{
	std::vector<std::string> args = parse_command(command);
	if(args.size() <= 0) {
		command.clear();

	} else if(args[0] == "menu") {
		main_page(client);
	} else if (args[0] == "riso") {
		risographie_page(client);
	} else if (args[0] == "connexion") {
		connexion_page(client);
	} else {
		client->minitel->moveCursorUp(1);
		client->minitel->moveCursorLeft(40);
		client->minitel->printChar('\r');
		client->minitel->print("COMMAND NOT HANDLE");
	}
	command.clear();
	machine_print_infos(client);
	return 0;
}
