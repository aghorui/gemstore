/*
 * networking.hpp - Networking Infrastructure.
 */

#ifndef GEM_NETWORKING_HPP
#define GEM_NETWORKING_HPP

#include <cstdint>
#include <mutex>
#include <queue>
#include <cstdlib>
#include <pthread.h>
#include <stdexcept>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.hpp"
#include "store.hpp"

namespace gem {

/**** NETWORKING DATA EXCHANGE STRUCTS ****************************************/

struct RootData {
	std::string nickname;
	uint64_t connected_peers;
};

static inline void to_json(json& j, const RootData& p) {
	j = json{
		{ "nickname",        p.nickname        },
		{ "connected_peers", p.connected_peers }
	};
}

static inline void from_json(const json& j, RootData& p) {
	j.at("nickname").get_to(p.nickname);
	j.at("connected_peers").get_to(p.connected_peers);
}

struct SyncStatusData {
	std::string hash;
	uint64_t last;
	uint64_t stamp;
};

struct SyncData {
	Vector<KeyValuePair> values;
};

static inline void to_json(json& j, const SyncStatusData& p) {
	j = json{
		{ "hash",  p.hash },
		{ "last",  p.last },
		{ "stamp", p.stamp }
	};
}

static inline void from_json(const json& j, SyncStatusData& p) {
	j.at("hash").get_to(p.hash);
	j.at("last").get_to(p.last);
	j.at("stamp").get_to(p.stamp);
}

struct KeyValueData {
	std::string key;
	json value;
};

static inline void to_json(json& j, const KeyValueData& p) {
	j = json{
		{ "key",  p.key },
		{ "value",  p.value }
	};
}

static inline void from_json(const json& j, KeyValueData& p) {
	j.at("key").get_to(p.key);
	j.at("value").get_to(p.value);
}

struct Query {
	enum QueryType {
		NORMAL,
		PATTERN
	};

	enum QueryAction {
		GET,
		SET,
		DELETE
	};

	QueryType query_type;
	QueryAction query_action;
	String key;
	String value;
};

#ifdef DUMMY_IMPLEMENTATION

using QueryResult = Vector<Value>;

#else
// TODO
/*
struct QueryResult {
	Store::ValueMap values;
};
*/
#endif

/******************************************************************************/

struct Server {
/*
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
*/

	Config &config;

	// All disk/mem i/o is done through this store interface.
	Store &store;

	int max_peers = 5;     // Max server peers the server can have
	int max_clients = 128; // Max clients the server will have connections with
	int concurrency = 32;  // Number of worker threads

	Set<PeerInformation> peer_list;

	httplib::Server peer_server;    // Server handler for peer connections
	httplib::Server client_server;  // Server handler for client connections

	uint16_t peer_port   = 4095; // Port for peer servers
	uint16_t client_port = 4096; // Port for clients

	using PushQueue = Vector<Key>;

	std::mutex push_queues_lock;
	Map<PeerInformation, PushQueue> push_queues;

/*
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
*/

	void start();
	void close();

	void handle_client_request();
	void handle_server_request();

	Server(Config &config, Store &store):
		config(config),
		store(store),
		concurrency(config.max_concurrency),
		peer_list(config.peers.begin(), config.peers.end()),
		peer_port(config.server_listener_port),
		client_port(config.client_listener_port)
/*		client_listener(ListenerThreadContext(
			client_connections,
			client_queue,
			client_queue_lock
		)),
		peer_listener(ListenerThreadContext(
			peer_connections,
			peer_queue,
			peer_queue_lock
		))*/ {}
};

struct ClientQueryError : public std::runtime_error {
	ClientQueryError(const std::string &message):
		std::runtime_error(message.c_str()) {}

	ClientQueryError(const httplib::Result &res):
		std::runtime_error(httplib::to_string(res.error())) {}

	ClientQueryError(const int &status):
		std::runtime_error(httplib::status_message(status)) {}
};

struct Client {
	String server_address;
	uint16_t client_port;
	uint16_t peer_port;
	httplib::Client client;
	httplib::Client peer_client;

	QueryResult get_value(std::string key);
	bool set_value(const std::string &key, const Value &value);
	json dump();

	Config peer_get_config();

	bool should_sync();
	SyncData peer_get_sync_changeset();

	Client(std::string url = "127.0.0.1",
	       int peer_port = GEM_DEFAULT_PEER_PORT,
	       int client_port = GEM_DEFAULT_CLIENT_PORT):
		client(url, client_port), peer_client(url, peer_port) {}
};

};

#endif
