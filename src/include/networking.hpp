/*
 * networking.hpp - Networking Infrastructure.
 */

#ifndef GEM_NETWORKING_HPP
#define GEM_NETWORKING_HPP

#include <condition_variable>
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

struct SyncData {
	PeerInformation peerinfo;
	Vector<KeyValuePair> values;
};

static inline void to_json(json& j, const SyncData& p) {
	j = json{
		{ "peerinfo", p.peerinfo },
		{ "values", p.values }
	};
}

static inline void from_json(const json& j, SyncData& p) {
	j.at("peerinfo").get_to(p.peerinfo);
	j.at("values").get_to(p.values);
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

	//Map<PeerInformation, PushQueue> push_queues;

	struct PeerState {
		PushQueue queue = {};
		bool accessed_once = false;
		int retries = 0;
		uint64_t last_pinged = 0;
		uint64_t last_updated = 0;
	};

	std::mutex peer_state_lock;
	Map<PeerInformation, PeerState> peers;

	std::condition_variable broadcast_queue_cond;
	// std::mutex broadcast_queue_lock;
	Queue<PeerInformation> broadcast_queue;

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
		client_port(config.client_listener_port) {
			for (auto pi : peer_list) {
				peers[pi] = PeerState();
			}
		}
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
	uint64_t client_port;
	uint64_t peer_port;
	httplib::Client client;
	httplib::Client peer_client;

	QueryResult get_value(std::string key);
	bool set_value(const std::string &key, const Value &value);
	json dump();

	Config peer_get_config();

	bool should_sync();
	SyncData peer_get_sync_changeset(uint peer_port_outgoing, uint client_port_outgoing);
	bool peer_send_changeset(SyncData &s);

	Client(std::string url = "127.0.0.1",
	       uint64_t peer_port = GEM_DEFAULT_PEER_PORT,
	       uint64_t client_port = GEM_DEFAULT_CLIENT_PORT):
		client_port(client_port), peer_port(peer_port),
		client(url, client_port), peer_client(url, peer_port) {}
};

};

#endif
