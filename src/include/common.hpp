#ifndef COMMON_HPP
#define COMMON_HPP

#include "json.hpp"
#include "httplib.h"
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <iostream>

#define GEMSTORE_VERSION "0.1"
#define GEMSTORE_NAME "gemstore"

namespace gem {

using json = nlohmann::json;

using String = std::string;

template<typename T>
using Queue = std::queue<T>;

template <typename K, typename V>
using Map = std::map<K, V>;

template <typename T>
using Set = std::set<T>;

template <typename T>
using Vector = std::vector<T>;

bool validate_json(String json_str);

struct PeerInformation {
	String address;
	uint16_t port;
};

static inline void to_json(json& j, const PeerInformation& p) {
	j = json{
		{"address", p.address},
		{"port", p.port}
	};
}

static inline void from_json(const json& j, PeerInformation& p) {
	j.at("address").get_to(p.address);
	j.at("port").get_to(p.port);
}

#define INSERT_CONFIG(name, type) { #name, type, &name }

struct Config {
	uint64_t client_listener_port = 4096;
	uint64_t server_listener_port = 4095;
	uint64_t max_server_connections = 20;
	uint64_t max_client_connections = 128;
	uint64_t max_concurrency = 30;
	std::vector<PeerInformation> peers;

	struct MappingTableEntry {
		const char *key;
		json::value_t type;
		void *value_ptr;
	};

	using MappingTable = std::vector<MappingTableEntry>;

	MappingTable table;

	Config() {
		table = {
			INSERT_CONFIG(client_listener_port, json::value_t::number_unsigned),
			INSERT_CONFIG(server_listener_port, json::value_t::number_unsigned),
			INSERT_CONFIG(max_server_connections, json::value_t::number_unsigned),
			INSERT_CONFIG(max_client_connections, json::value_t::number_unsigned),
			INSERT_CONFIG(max_concurrency, json::value_t::array),
		};
	}
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