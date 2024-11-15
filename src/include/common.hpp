/*
 * common.hpp - Common Components
 */

#ifndef GEM_COMMON_HPP
#define GEM_COMMON_HPP
#include <chrono>
#include <deque>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <unordered_map>

#include "json.hpp"
#include "httplib.h"

#define GEMSTORE_VERSION "0.2"
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

using Key = String;

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

enum class SyncMode {
	POLL,
	BROADCAST
};

#define SYNCMODE_TO_STR(x) (((x) == SyncMode::POLL) ? "poll" : \
                           (((x) == SyncMode::BROADCAST) ? "broadcast" : ""))
#define STR_TO_SYNCMODE(x) (((x) == "poll") ? SyncMode::POLL : \
                           (((x) == "broadcast") ? SyncMode::BROADCAST : SyncMode::POLL))

static inline void to_json(json& j, const SyncMode& p) {
	std::cerr << "SYNCMODE VALUE to json: " << SYNCMODE_TO_STR(p) ;
	if (p == SyncMode::POLL) {
		j = "poll";
	} else if (p == SyncMode::BROADCAST) {
		j = "broadcast";
	} else {
		throw std::invalid_argument("Sync mode is neither 'poll' or 'broadcast'");
	}
}

static inline void from_json(const json& j, SyncMode& p) {
	std::cerr << "SYNCMODE VALUE from json: " << j.dump();
	if (j == "poll") {
		p = SyncMode::POLL;
	} else if (j == "broadcast") {
		p = SyncMode::BROADCAST;
	} else {
		throw std::invalid_argument("Sync mode is neither 'poll' or 'broadcast'");
	}
}

enum MergeAttributes {
	NUM_NONE,
	NUM_MIN,
	NUM_MAX,
	NUM_AVERAGE,
	STR_NONE,
	STR_CONCAT,
	ARR_NONE,
	ARR_CONCAT,
	ARR_UNION
};

const Map<MergeAttributes, String> merge_attr_to_str = {
	{ NUM_NONE,      "num_none"    },
	{ NUM_MIN,       "num_min"     },
	{ NUM_MAX,       "num_max"     },
	{ NUM_AVERAGE,   "num_average" },
	{ STR_NONE,      "str_none"    },
	{ STR_CONCAT,    "str_concat"  },
	{ ARR_NONE,      "arr_none"    },
	{ ARR_CONCAT,    "arr_concat"  },
	{ ARR_UNION,     "arr_union"   }
};

const Map<String, MergeAttributes> str_to_merge_attr = {
	{ "num_none",    NUM_NONE      },
	{ "num_min",     NUM_MIN       },
	{ "num_max",     NUM_MAX       },
	{ "num_average", NUM_AVERAGE   },
	{ "str_none",    STR_NONE      },
	{ "str_concat",  STR_CONCAT    },
	{ "arr_none",    ARR_NONE      },
	{ "arr_concat",  ARR_CONCAT    },
	{ "arr_union",   ARR_UNION     }
};

struct MergeAttributeEntry {
	Key key;
	MergeAttributes attr;
};

static inline void to_json(json& j, const MergeAttributeEntry& attr) {
	auto val = merge_attr_to_str.find(attr.attr);
	if (val == merge_attr_to_str.end()) {
		throw std::invalid_argument("Unknown merge attribute.");
	}
	j = json{
		{ "key",  attr.key },
		{ "attr", val->second },
	};
}

static inline void from_json(const json& j, MergeAttributeEntry& attr) {
	auto val = str_to_merge_attr.find(j.at("attr"));
	if (val == str_to_merge_attr.end()) {
		throw std::invalid_argument("Unknown merge attribute in JSON.");
	}

	j.at("key").get_to(attr.key);
	attr.attr = val->second;
}


struct Config {
	uint64_t client_listener_port = 4096;
	uint64_t server_listener_port = 4095;
	uint64_t max_server_connections = 20;
	uint64_t max_client_connections = 128;
	uint64_t max_concurrency = 30;
	uint64_t max_peers = 10;
	float    peer_retention = 0.5;
	SyncMode sync_mode = SyncMode::POLL;
	Vector<MergeAttributeEntry> merge_attributes;
	Vector<PeerInformation> peers;
};

static inline void to_json(json& j, const Config& p) {
	j = json{
		{ "client_listener_port",   p.client_listener_port       },
		{ "server_listener_port",   p.server_listener_port       },
		{ "max_server_connections", p.max_server_connections     },
		{ "max_client_connections", p.max_client_connections     },
		{ "max_concurrency",        p.max_concurrency            },
		{ "sync_mode",              p.sync_mode                  },
		{ "peer_retention",         p.peer_retention             },
		{ "peers",                  p.peers                      },
		{ "merge_attributes",       p.merge_attributes           }
	};
}

static inline void from_json(const json& j, Config& p) {
	j.at("client_listener_port").get_to(p.client_listener_port);
	j.at("server_listener_port").get_to(p.server_listener_port);
	j.at("max_server_connections").get_to(p.max_server_connections);
	j.at("max_client_connections").get_to(p.max_client_connections);
	j.at("max_concurrency").get_to(p.max_concurrency);
	j.at("peers").get_to(p.peers);
	j.at("merge_attributes").get_to(p.merge_attributes);
	j.at("sync_mode").get_to(p.sync_mode);
}

static inline uint64_t get_millisecond_timestamp() {
	return std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
}

};

#endif