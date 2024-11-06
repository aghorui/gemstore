#include <ctime>
#include <iomanip>
#include <iostream>
#include "common.hpp"
#include "httplib.h"

#include "networking.hpp"


namespace gem {

// void Server::peer_worker_func(WorkerThreadContext &ctx);
// void Server::client_worker_func(WorkerThreadContext &ctx);
// void Server::client_listener_func(ListenerThreadContext &ctx);

/*void Server::peer_listener_func(ListenerThreadContext &ctx) {
	int sockfd = start_server()

	int epollfd = epoll_create1(0);

	if (epollfd < 0) {
		perror("epoll_create1");
		exit(1);
	}

	struct epoll_event listener_fd
}*/

void print_request(const httplib::Request &req) {
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::cerr << "[" << std::put_time(&tm, "%Y-%m-%d %H-%M-%S") << "] "
	          << req.method << " " << req.target
	          << " from " << req.remote_addr << ":" << req.remote_port
	          << std::endl;
}

void root_handler(gem::Server &, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	resp.set_content("pong", "text/plain");
}

void sync_get_handler(gem::Server &, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	json j = {
		{ "hash", "23j12j3i2w" },
		{ "last", 3123123232323 },
		{ "stamp", 3343 }
	};
	resp.set_content(j.dump(), "application/json");
}

void config_get_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	json j;
	to_json(j, server.config);
	resp.set_content(j.dump(), "application/json");
}

#define HANDLER(__handler_func) \
	[&](const httplib::Request &req, httplib::Response &resp) { \
		__handler_func((*this), req, resp); \
	}

void Server::start() {

	peer_server.Get("/", HANDLER(root_handler));
	peer_server.Get("/sync", HANDLER(sync_get_handler));
	peer_server.Get("/config", HANDLER(config_get_handler));

	std::cerr << "Starting server at 0.0.0.0:" << peer_port << std::endl;

	if (!peer_server.bind_to_port("0.0.0.0", peer_port)) {
		std::cerr << "Could not bind to port." << std::endl;
		return;
	}

	if (!peer_server.listen_after_bind()) {
		std::cerr << "Could not listen on socket." << std::endl;
		return;
	}

	std::cerr << "Exited." << std::endl;
}

void Server::close() {
	peer_server.stop();
	client_server.stop();
}

};