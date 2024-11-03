/*
 * networking.hpp - Networking Infrastructure.
 */

#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <cstdint>
#include <queue>
#include <pthread.h>

#include "common.hpp"
#include "store.hpp"

namespace gem {

struct ServerRequest {
	enum RequestCreatorType {
		PEER,
		CLIENT
	};

	String sender_address;
	uint16_t sender_port;

};

struct Server {
	// All disk/mem i/o is done through this store interface.
	Store &store;

	int max_peers = 5;     // Max server peers the server can have
	int max_clients = 128; // Max clients the server will have connections with
	int concurrency = 32;  // Number of worker threads

	uint16_t port;

	pthread_mutex_t queue_lock;

	// Request Queue.
	Queue<ServerRequest> queue;

	void start_listener();

	void handle_client_request();
	void handle_server_request();

	Server(Store &s): store(s) {}
};




struct Client {
	String server_address;
	uint16_t port;

	void connect();
	QueryResult send_query();
};

};

#endif
