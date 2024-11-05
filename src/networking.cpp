#include <iostream>
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

void basic_handler(const httplib::Request &req, httplib::Response &resp) {
	std::cout << "Request from: "
	          << req.remote_addr << ":"
	          << req.remote_port << std::endl;

	resp.set_content("({\"status\": \"success\"})", "application/json");
}

void Server::start() {

	peer_server.Get("/", basic_handler);

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