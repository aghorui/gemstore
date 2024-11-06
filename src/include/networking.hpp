/*
 * networking.hpp - Networking Infrastructure.
 */

#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <cstdint>
#include <queue>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.hpp"
#include "store.hpp"


namespace gem {

struct Connection {
	enum ConnectionType {
		PEER,
		CLIENT
	};

	ConnectionType connection_type;
	String sender_address;
	uint16_t sender_port;

	int fd;

	void close() {
		::close(fd);
	}
};

struct Request {
	enum RequestType {
		READ,
		WRITE
	};

	RequestType request_type;
	Connection &requester;
};

struct Server;

struct Server {

	template<typename ContextT, void (*thread_func)(ContextT &)>
	static void *worker_thread_func(void *arg) {
		ContextT *context = (ContextT *) arg;
		thread_func(context);
		return NULL;
	}

	template<typename ContextT, void (*thread_func)(ContextT &)>
	struct WorkerThread {
		pthread_t thread;
		ContextT context;

		void start() {
			pthread_create(&thread, NULL,
				worker_thread_func<ContextT, thread_func>,
				&context);
		}

		void stop() {
			context.done = true;
			pthread_join(thread, NULL);
		}

		WorkerThread(ContextT ctx): context(ctx) {}
	};

	struct WorkerThreadContext {
		int id = -1;
		bool done = false;
		Store &store;
		Queue<Request> &queue;
		pthread_mutex_t &queue_lock;

		WorkerThreadContext(
			Store &store,
			Queue<Request> &queue,
			pthread_mutex_t &queue_lock):
			store(store),
			queue(queue),
			queue_lock(queue_lock) {}
	};

	struct ListenerThreadContext {
		int id = -1;
		uint16_t port;
		bool done = false;
		Set<Connection> &connections;
		Queue<Request> &queue;
		pthread_mutex_t &queue_lock;

		ListenerThreadContext(
			Set<Connection> &connections,
			Queue<Request> &queue,
			pthread_mutex_t &queue_lock):
			connections(connections),
			queue(queue),
			queue_lock(queue_lock) {}
	};

	// static void peer_worker_func(WorkerThreadContext &ctx);
	// static void client_worker_func(WorkerThreadContext &ctx);
	static void peer_listener_func(ListenerThreadContext &ctx);
	// static void client_listener_func(ListenerThreadContext &ctx);

	using PeerListenerThread = WorkerThread<ListenerThreadContext, peer_listener_func>;
	// using ClientListenerThread = WorkerThread<ListenerThreadContext, client_listener_func>;
	// using ClientWorkerThread = WorkerThread<WorkerThreadContext, client_worker_func>;
	// using PeerWorkerThread = WorkerThread<WorkerThreadContext, peer_worker_func>;


	Config &config;

	// All disk/mem i/o is done through this store interface.
	Store &store;

	int max_peers = 5;     // Max server peers the server can have
	int max_clients = 128; // Max clients the server will have connections with
	int concurrency = 32;  // Number of worker threads

	Vector<PeerInformation> peer_list;

	httplib::Server peer_server;    // Server handler for peer connections
	httplib::Server client_server;  // Server handler for client connections

	uint16_t peer_port   = 4095; // Port for peer servers
	uint16_t client_port = 4096; // Port for clients

	Set<Connection> peer_connections;
	Set<Connection> client_connections;

	// Peer Request Queue.
	Queue<Request> peer_queue;
	pthread_mutex_t peer_queue_lock;


	// Client Request Queue.
	Queue<Request> client_queue;
	pthread_mutex_t client_queue_lock;

	// ClientListenerThread client_listener;
	PeerListenerThread peer_listener;

	//Vector<ClientWorkerThread> client_worker_threads;
	//Vector<PeerWorkerThread> peer_worker_threads;

	void start();
	void close();

	void handle_client_request();
	void handle_server_request();

	Server(Config &config, Store &store, Vector<PeerInformation> peers = {}):
		config(config),
		store(store),
		peer_list(peers.begin(), peers.end()),
/*		client_listener(ListenerThreadContext(
			client_connections,
			client_queue,
			client_queue_lock
		)),*/
		peer_listener(ListenerThreadContext(
			peer_connections,
			peer_queue,
			peer_queue_lock
		)) {}
};

// struct Client {
// 	String server_address;
// 	uint16_t port;

// 	void connect();
// 	void close();
// 	QueryResult send_query();
// };

};

#endif
