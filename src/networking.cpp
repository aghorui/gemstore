#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <thread>
#include "common.hpp"
#include "httplib.h"

#include "networking.hpp"
#include "store.hpp"


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
	log() << req.method << " " << req.target << " from "
	      << req.remote_addr << ":" << req.remote_port;
}

inline std::string err_msg(const std::string &msg) {
	return json{
		{ "message", msg }
	}.dump();
}

/**** SYNC IMPLEMENTATION *****************************************************/

enum class OperationType {
	CREATE,
	SET,
	DELETE,
	SETATTR
};

struct HistoryEntry {
	uint64_t timestamp;
	uint64_t index;
	uint64_t predecessor;
	uint64_t blame;
	OperationType operation;
	std::string key;
	Value value;
	std::vector<MergeAttributes> attrs;
};

struct History {
	Queue<HistoryEntry> list;
	Map<uint64_t, uint64_t> peer_indexes;
	uint64_t stamp_autonumber = 0;
	uint64_t queue_limit = 1024;

	void add_transaction(
		OperationType operation,
		const std::string &key,
		Value &v, std::vector<MergeAttributes> attrs = {});

	void merge_history(
		std::vector<HistoryEntry> merge_list,
		Store &s);

	bool need_full_sync(uint64_t timestamp, uint64_t index);

	uint64_t sync_start_index();
};

// milliseconds
#define POLL_DELAY 500

void sync_worker_poll(gem::Server &server) {
	// ping each server

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(POLL_DELAY));

			for (auto &p : server.peer_list) {
				try {
					Client peer_client(p.address, p.peer_port, p.client_port);
					//log() << "Pinging " << p.address << ":" << p.peer_port;
					SyncData s = peer_client.peer_get_sync_changeset(server.peer_port, server.client_port);

					if (s.values.size() > 0) {
						log() << "Update of size " << s.values.size() << " from "
						      << p.address << ":" << p.peer_port;

						// Update self
						server.store.bulk_update(s.values);

						// Mark dirty to other peers
						server.peer_state_lock.lock();

						for (auto &peer : server.peers) {
							log() << peer.first.to_string() << " " << p.to_string();
							if (peer.first == p) {
								log() << "EQUAL FOUND, Skipping";
								continue;
							}
							for (auto &k : s.values) {
								peer.second.queue.push_back(k.key);
							}
						}

						server.peer_state_lock.unlock();
					}


				} catch (std::exception &e) {
					log() << "Error on pinging "
					      << p.address << ":" << p.peer_port << ": " << e.what();
				}
			}
	}
}

void sync_worker_broadcast(gem::Server &server) {
	// broadcast to each server

	while (true) {
		// std::this_thread::sleep_for(std::chrono::milliseconds(POLL_DELAY));
		std::unique_lock<std::mutex> peer_state_unique_lock(server.peer_state_lock);

		server.broadcast_queue_cond.wait(
			peer_state_unique_lock,
			[&] { return server.broadcast_queue.size() > 0; });

		PeerInformation p = server.broadcast_queue.front();
		server.broadcast_queue.pop_front();

		SyncData s;
		s.peerinfo.address = "";
		s.peerinfo.peer_port = server.peer_port;
		s.peerinfo.client_port = server.client_port;

		s.values = server.store.bulk_get(server.peers[p].queue);

		Client client(p.address, p.peer_port, p.client_port);
		try {
			client.peer_send_changeset(s);
		} catch (ClientQueryError &e) {
			log() << "Broadcast Error: " << e.what();
		}
	}
}

/**** SERVER IMPLEMENTATION ***************************************************/

void root_handler(gem::Server &, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	RootData r;
	r.nickname = "acd";
	r.connected_peers = 20;
	resp.set_content(json(r).dump(), "application/json");
}

void broadcast_sync_post_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	SyncData s;

	try {
		s = json::parse(req.body);
	} catch (json::parse_error &e) {
		resp.status = httplib::BadRequest_400;
		resp.set_content(err_msg("malformed request"), "application/json");
		return;
	}

	PeerInformation sender_peer;
	sender_peer.address = req.local_addr;
	sender_peer.peer_port = s.peerinfo.peer_port;
	sender_peer.client_port = s.peerinfo.client_port;

	server.store.bulk_update(s.values);

	server.peer_state_lock.lock();

	for (auto &peer : server.peers) {
		log() << peer.first.to_string() << " " << sender_peer.to_string();
		if (peer.first == sender_peer) {
			log() << "EQUAL FOUND, Skipping";
			continue;
		}
		for (auto &k : s.values) {
			peer.second.queue.push_back(k.key);
		}

		server.broadcast_queue.push_back(peer.first);
	}

	server.peer_state_lock.unlock();
	server.broadcast_queue_cond.notify_all();

	resp.status = httplib::OK_200;
}

void sync_post_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	//print_request(req);
	PeerInformation p;

	try {
		p = json::parse(req.body);
	} catch (json::parse_error &e) {
		resp.status = httplib::BadRequest_400;
		resp.set_content(err_msg("malformed request"), "application/json");
		return;
	}

	p.address = req.local_addr;
	// log() << "Request from " << json(p).dump(4);
	// log() << "Peer list is " << json(server.peer_list).dump(4);
	if (server.peer_list.count(p) < 1) {
		resp.status = httplib::Forbidden_403;
		resp.set_content(err_msg("not a peer"), "application/json");
		return;
	}

	bool force_resync = false;
	std::string force_resync_str = req.get_header_value("Gem-Force-Resync");
	if (force_resync_str == "true") {
		force_resync = true;
	}

	server.peer_state_lock.lock();

	if (server.peers.count(p) < 1) {
		server.peers[p] = Server::PeerState();
	}

	if (server.peers[p].accessed_once == false || force_resync) {
		// new request
		server.peers[p].accessed_once = true;
		resp.status = httplib::OK_200;
		resp.set_header("Gem-Force-Resync", "true");
		resp.set_content(server.store.dump().dump(), "application/json");
	} else {
		SyncData s;
		s.peerinfo.address = "";
		s.peerinfo.peer_port = server.peer_port;
		s.peerinfo.client_port = server.client_port;
		s.values = server.store.bulk_get(server.peers[p].queue);
		resp.status = httplib::OK_200;
		resp.set_content(json(s).dump(), "application/json");
		server.peers[p].queue.clear();
	}

	server.peer_state_lock.unlock();
}

void config_get_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	resp.set_content(json(server.config).dump(), "application/json");
}

void client_query_get_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	std::string query = req.get_param_value("q");

	if (query.empty()) {
		resp.status = httplib::BadRequest_400;
		resp.set_content(err_msg("no parameter 'q' specified"), "application/json");
		return;
	} else {
		Value v;

		try {
			v = server.store.get(query);
		} catch (Store::KeyNotFoundException&) {
			resp.status = httplib::NotFound_404;
			resp.set_content(err_msg("key not found"), "application/json");
			return;
		}

		KeyValueData k;
		k.key = query;
		k.value = v.to_json_value();

		resp.status = httplib::OK_200;
		resp.set_content(json(k).dump(), "application/json");
		return;
	}
}

void client_query_set_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	KeyValueData k;
	/*std::string key = req.get_param_value("key");
	std::string value_str = req.get_param_value("value");
	json value;

	if (key.empty() || value.empty()) {
		resp.status = httplib::BadRequest_400;
		resp.set_content(
			err_msg("'key' or 'value' not specified"),
			"application/json");
		return;
	}*/

	try {
		k = json::parse(req.body);
	} catch (json::parse_error&) {
		resp.status = httplib::BadRequest_400;
		resp.set_content(
			err_msg("specified value is not valid json"),
			"application/json");
		return;
	}

	if (k.value.is_object()) {
		resp.status = httplib::NotImplemented_501;
		resp.set_content(
			err_msg("object storage is not implemented yet"),
			"application/json");
		return;
	}

	Value in_value;
	in_value.type = get_type_from_json(k.value);
	in_value.storage = k.value;

	if (!server.store.set(k.key, in_value)) {
		resp.status = httplib::NotImplemented_501;
		resp.set_content(
			err_msg("object storage is not implemented yet"),
			"application/json");
		return;
	}

	server.peer_state_lock.lock();

	for (auto &peer : server.peers) {
		peer.second.queue.push_back(k.key);
	}

	server.peer_state_lock.unlock();

	resp.status = httplib::OK_200;
}

void client_dump_handler(gem::Server &server, const httplib::Request &req, httplib::Response &resp) {
	print_request(req);
	resp.status = httplib::OK_200;
	resp.set_content(server.store.dump().dump(), "application/json");
}

void exception_handler(const httplib::Request &req, httplib::Response &resp, std::exception_ptr ep) {
	if (!ep) {
		log() << "INTERNAL ERROR WITH BAD EXCPETION ON "
		      << req.method << " " << req.target << " from "
		      << req.remote_addr << ":" << req.remote_port;
		resp.status = httplib::InternalServerError_500;
	}

	try {
		std::rethrow_exception(ep);
	} catch (std::exception &e) {
		log() << "INTERNAL ERROR ON " << req.method << " " << req.target << " from "
		      << req.remote_addr << ":" << req.remote_port << " : " << e.what();
		resp.status = httplib::InternalServerError_500;
	}
}

void enable_cors(const httplib::Request &req, httplib::Response &resp) {
	resp.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin"));
	resp.set_header("Allow", "GET, POST, HEAD, DELETE");
	resp.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");
	resp.set_header("Access-Control-Allow-Methods", "GET, POST, HEAD, DELETE");
}

#define HANDLER(__handler_func) \
	([&](const httplib::Request &req, httplib::Response &resp) { \
		enable_cors(req, resp); \
		__handler_func((*this), req, resp); \
	})

void Server::start() {
	peer_server.Get("/", HANDLER(root_handler));
	peer_server.Post("/sync", HANDLER(sync_post_handler));

	if (config.sync_mode == SyncMode::BROADCAST) {
		peer_server.Post("/broadcast_sync", HANDLER(sync_post_handler));
	}

	peer_server.Get("/config", HANDLER(config_get_handler));
	// peer_server.Options("/config", HANDLER(enable_cors));

	peer_server.set_exception_handler(exception_handler);

	client_server.Get("/", HANDLER(root_handler));
	// client_server.Options("/", HANDLER(enable_cors));

	client_server.Get("/query", HANDLER(client_query_get_handler));
	// client_server.Options("/query", HANDLER(enable_cors));

	client_server.Post("/set", HANDLER(client_query_set_handler));
	client_server.Options("/set", enable_cors);

	client_server.Get("/dump", HANDLER(client_dump_handler));
	// client_server.Options("/dump", HANDLER(enable_cors));

	client_server.set_exception_handler(exception_handler);

	std::thread peer_server_thread([&]{
		log() << "Starting peer server at http://127.0.0.1:" << peer_port;

		if (!peer_server.bind_to_port(GEM_DEFAULT_SERVER_ADDRESS, peer_port)) {
			log() << "[peer_server] Could not bind to port.";
			exit(1);
		}

		if (!peer_server.listen_after_bind()) {
			log() << "[peer_server] Could not listen on socket.";
			exit(1);
		}
	});

	std::thread client_server_thread([&]{
		log() << "Starting client server at http://127.0.0.1:" << client_port;

		if (!client_server.bind_to_port(GEM_DEFAULT_SERVER_ADDRESS, client_port)) {
			log() << "[client_server] Could not bind to port.";
			exit(1);
		}

		if (!client_server.listen_after_bind()) {
			log() << "[client_server] Could not listen on socket.";
			exit(1);
		}
	});

	std::thread sync_thread;

	if (peer_list.size() > 0) {
		if (config.sync_mode == SyncMode::POLL) {
			sync_thread = std::thread([&] { sync_worker_poll(*this); });
		} else if (config.sync_mode == SyncMode::BROADCAST) {
			sync_thread = std::thread([&] { sync_worker_broadcast(*this); });
		} else {
			log() << "Sync mode is neither poll or broadcast. Illegal state";
			exit(1);
		}
	}

	peer_server_thread.join();
	client_server_thread.join();
}

void Server::close() {
	peer_server.stop();
	client_server.stop();
}

/**** CLIENT IMPLEMENTATION ***************************************************/

QueryResult Client::get_value(std::string key)  {
	Value v;

	httplib::Params params;
	params.emplace("q", key);

	auto res = client.Get(httplib::append_query_params("/query", params));

	if (!res) {
		throw ClientQueryError(res);
	}

	if (res->status != httplib::OK_200) {
		throw ClientQueryError(res->status);
	}

	KeyValueData k = json::parse(res->body);

	v.type = get_type_from_json(k.value);
	v.storage = k.value;

	return {v};
}

bool Client::set_value(const std::string &key, const Value &value) {
	KeyValueData k;
	k.key = key;
	k.value = value.to_json_value();
	// log() << "Client Config: " << client.host() << " " << client.port();
	// log() << "Peer Client Config: " << peer_client.host() << " " << peer_client.port();
	auto res = client.Post("/set", json(k).dump(), "application/json");

	if (!res) {
		throw ClientQueryError(res);
	}

	if (res->status != httplib::OK_200) {
		throw ClientQueryError(res->status);
	}

	return true;
}

Config Client::peer_get_config() {
	auto res = peer_client.Get("/config");

	if (!res) {
		throw ClientQueryError(res);
	}

	if (res->status != httplib::OK_200) {
		throw ClientQueryError(res->status);
	}

	Config c = json::parse(res->body);

	return c;
}

SyncData Client::peer_get_sync_changeset(uint peer_port_outgoing, uint client_port_outgoing) {
	PeerInformation p;
	p.address = "127.0.0.1"; // PLACEHOLDER
	p.peer_port = peer_port_outgoing;
	p.client_port = client_port_outgoing;

	// log() << "SENDING request: " << json(p).dump(4);

	auto res = peer_client.Post("/sync",
		json(p).dump(), "application/json");

	if (!res) {
		throw ClientQueryError(res);
	}

	if (res->status != httplib::OK_200) {
		throw ClientQueryError(res->status);
	}

	SyncData s;

	if (res->has_header("Gem-Force-Resync") &&
		res->get_header_value("Gem-Force-Resync") == "true") {
		json j = json::parse(res->body);
		// Unelegant.
		for (auto &k : j.items()) {
			KeyValuePair kv;
			kv.key = k.key();
			kv.value.type = get_type_from_json(k.value());
			kv.value.storage = k.value();
			s.values.push_back(kv);
		}
	} else {
		s = json::parse(res->body);
	}

	return s;
}

bool Client::peer_send_changeset(SyncData &s) {
	auto res = peer_client.Post("/broadcast_sync",json(s).dump(), "application/json");

	if (!res) {
		throw ClientQueryError(res);
	}

	if (res->status != httplib::OK_200) {
		throw ClientQueryError(res->status);
	}

	return true;
}

json Client::dump() {
	auto res = client.Get("/dump");

	if (!res) {
		throw ClientQueryError(res);
	}

	if (res->status != httplib::OK_200) {
		throw ClientQueryError(res->status);
	}

	return json::parse(res->body);
}

};