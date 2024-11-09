/*
 * common.hpp - Common Components
 */

#ifndef GEM_COMMON_HPP
#define GEM_COMMON_HPP

#include "json.hpp"
#include "httplib.h"
#include <deque>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <iostream>

#define GEMSTORE_VERSION "0.1"
#define GEMSTORE_NAME "gemstore"

#define GEM_DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define GEM_DEFAULT_CLIENT_PORT 4095
#define GEM_DEFAULT_PEER_PORT 4096

#define DUMMY_IMPLEMENTATION

namespace gem {

using json = nlohmann::json;

using String = std::string;

template<typename T>
using Queue = std::deque<T>;

template <typename K, typename V>
using Map = std::map<K, V>;

template <typename T>
using Set = std::set<T>;

template <typename T>
using Vector = std::vector<T>;

bool validate_json(String json_str);

/**
 * @brief      Basic logging class. use this instead of std::cout and std::cerr
 */
class log {
private:
	std::ostringstream os;
	std::ostream &stream;

public:
	// Used to handle cases like std::endl
	using StreamFunction = std::ostream&(*)(std::ostream&);

	template <typename T>
	log& operator<<(T const& t) {
		os << t;
		return *this;
	}

	log& operator<<(StreamFunction f) {
		os << f;
		return *this;
	}

	log(std::ostream &stream = std::cerr): stream(stream) {
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		os << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
	}

	~log() {
		stream << os.str() << std::endl;
	}
};

struct PeerInformation {
	String address;
	uint16_t peer_port;
	uint16_t client_port;

	bool operator<(const PeerInformation &o) const {
		return (address < o.address) ||
		       (peer_port < o.peer_port) ||
		       (client_port < o.client_port);
	}

	bool operator==(const PeerInformation &o) const {
		return (address == o.address) &&
		       (peer_port == o.peer_port) &&
		       (client_port == o.client_port);
	}

	std::string to_string() const {
		return "PeerInformation{" + address + " p: " + std::to_string(peer_port)
		       + " c: " + std::to_string(client_port) + "}";
	}
};

static inline void to_json(json& j, const PeerInformation& p) {
	j = json{
		{"address", p.address},
		{"peer_port", p.peer_port},
		{"client_port", p.client_port}
	};
}

static inline void from_json(const json& j, PeerInformation& p) {
	j.at("address").get_to(p.address);
	j.at("peer_port").get_to(p.peer_port);
	j.at("client_port").get_to(p.client_port);
}

#define INSERT_CONFIG(name, type) { #name, type, &name }

struct Config {
	uint64_t client_listener_port = 4096;
	uint64_t server_listener_port = 4095;
	uint64_t max_server_connections = 20;
	uint64_t max_client_connections = 128;
	uint64_t max_concurrency = 30;
	std::vector<PeerInformation> peers;
};

static inline void to_json(json& j, const Config& p) {
	j = json{
		{ "client_listener_port",   p.client_listener_port   },
		{ "server_listener_port",   p.server_listener_port   },
		{ "max_server_connections", p.max_server_connections },
		{ "max_client_connections", p.max_client_connections },
		{ "max_concurrency",        p.max_concurrency        },
		{ "peers",                  p.peers                  }
	};
}

static inline void from_json(const json& j, Config& p) {
	j.at("client_listener_port").get_to(p.client_listener_port);
	j.at("server_listener_port").get_to(p.server_listener_port);
	j.at("max_server_connections").get_to(p.max_server_connections);
	j.at("max_client_connections").get_to(p.max_client_connections);
	j.at("max_concurrency").get_to(p.max_concurrency);
	j.at("peers").get_to(p.peers);
}

};

#endif