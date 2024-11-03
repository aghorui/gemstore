#include <iostream>
#include "httplib.h"

#include "networking.hpp"


namespace gem {

const char *get_address(struct sockaddr *s, char *buf, int *port, int buf_len) {
	const char *addr = NULL;

	switch(s->sa_family) {
	case AF_INET:
		addr = inet_ntop(
			s->sa_family,
			&(((struct sockaddr_in *) s)->sin_addr),
			buf,
			buf_len
		);
		*port = (((struct sockaddr_in *) s)->sin_port);
		break;
	case AF_INET6:
		addr = inet_ntop(
			s->sa_family,
			&(((struct sockaddr_in6 *) s)->sin6_addr),
			buf,
			buf_len
		);
		*port = (((struct sockaddr_in6 *) s)->sin6_port);
		break;

	case AF_UNSPEC:
		fprintf(stderr, "Could not determine IP address.\n");
		break;
	};

	return addr;
}

int start_server(uint16_t port, std::string &ip_addr_dest) {
	int sockfd;
	int status;
	int yes = 1;

	struct addrinfo hints;      // Supplied to getaddrinfo as connection options
	struct addrinfo *servinfo;  // Received server connection information
	struct addrinfo *p;         // Pointer to traverse linked list in servinfo.

	memset(&hints, 0, sizeof(hints)); // We first clear the structure

	// Then we say that IPv4/IPv6 family is unspecified. We do not care what
	// protocol we use
	hints.ai_family = AF_UNSPEC;

	// Then we say that this is a TCP connection
	hints.ai_socktype = SOCK_STREAM;

	// We will ask the system to automatically give us an IP address (which is
	// of the system)
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		// Attempt to create a socket and store its file descriptor.
		// Go to the next addrinfo struct on error.
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "Error opening socket %d.\n", sockfd);
			continue;
		}

		// Try to reuse an port that was previously occupied and appears to be
		// still "plugged in".
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
			fprintf(stderr, "Error: could not manipulate port.\n");
			return -1;
		}

		// Now attempt to bind to this port and address' socket. Continue to the
		// next one on error.
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			fprintf(stderr, "Error: could not bind to port.\n");
			continue;
		} else {
			// Socket opening successful.
			break;
		}
	}

	// Test whether binding was successful or not.
	// i.e. We have gone to the end of the linked list.
	if (p == NULL) {
		freeaddrinfo(servinfo);
		fprintf(stderr, "Error: connection failed.\n");
		return -1;
	}

	int ret_port;
	char ip_addr_buf[INET6_ADDRSTRLEN];
	const char *addr = get_address(p->ai_addr, ip_addr_buf, &ret_port, INET6_ADDRSTRLEN);
	ip_addr_dest = ip_addr_buf;

	if (addr == NULL) {
		perror("inet_ntop");
		return -1;
	}

	// We are done. Cleanup.
	p = NULL;
	freeaddrinfo(servinfo);

	// Return server fd.
	return sockfd;
}


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

void Server::start() {

	peer_server.Get("/", [&](const httplib::Request &req, httplib::Response &resp) {
		std::cout << "Request from: "
		          << req.remote_addr << ":"
		          << req.remote_port << std::endl;

		resp.set_content(R"(
		{
			"status": "success"
		}
		)", "text/json");
	});

	peer_server.listen("0.0.0.0", peer_port);
}

void Server::close() {
	peer_server.stop();
	client_server.stop();
}

};