#include "networking.hpp"
#include "store.hpp"

const char *help_message_header = GEMSTORE_NAME " " GEMSTORE_VERSION R"(

This is a reference client for gemstore for showcasing the c++ API usage.
The following commands are available for you to use:

)";

const char *help_message_footer = R"(
"SERVINFO" is optional server info that can be passed to the set and get
commands. It is specified as follows:

  <server url> <peer port> <client_port>

All three must be specified. Otherwise the default tuple will be taken as:

  127.0.0.1 4095 4096
)";

void print_help_message(const char *execname) {
	std::cerr << help_message_header
	<< "  " << execname << " [help/-h/--help]              " << "Print this information\n"
	<< "  " << execname << " [-v/--version]                " << "Print version information\n"
	<< "  " << execname << " set <key> <value> [SERVINFO]  " << "Set a key\n"
	<< "  " << execname << " get <key> [SERVINFO]          " << "Get a key\n"
	<< "  " << execname << " configinfo [SERVINFO]         " << "Gets the config info for the server\n"
	//<< "  " << execname << " syncinfo [SERVINFO]           " << "Gets the sync info for the server\n"
	<< "  " << execname << " dump [SERVINFO]               " << "Dump all server data\n"
	<< help_message_footer;
}

struct ServerInfo {
	const char *address = "127.0.0.1";
	int peer_port = 4095;
	int client_port = 4096;
};

ServerInfo get_server_info(char **argv) {
	ServerInfo s;
	s.address = argv[0];
	s.peer_port = atoi(argv[1]);
	s.client_port = atoi(argv[2]);

	return s;
}

#define BASE_COMMAND 1
#define COMMAND_ARG_1 2
#define COMMAND_ARG_2 3
#define SERVINFO_2_OFFSET (COMMAND_ARG_2 + 1)
#define SERVINFO_1_OFFSET (COMMAND_ARG_1 + 1)
#define SERVINFO_0_OFFSET (BASE_COMMAND + 1)

int main(int argc, char **argv) {
	if (argc < 2) {
		print_help_message(argv[0]);
		exit(0);
	}

	if (strcmp(argv[1], "help") == 0 ||
		strcmp(argv[1], "-h") == 0 ||
		strcmp(argv[1], "--help") == 0) {
		print_help_message(argv[0]);
		exit(0);
	}

	if (strcmp(argv[1], "-v") == 0 ||
		strcmp(argv[1], "--version") == 0) {
		std::cerr << GEMSTORE_NAME " version " GEMSTORE_VERSION << std::endl;
		exit(0);
	}

	ServerInfo s;

	if (strcmp(argv[1], "set") == 0) {
		if (argc < COMMAND_ARG_2 + 1) {
			std::cerr << "Error: set requires at least 2 arguments." << std::endl;
			exit(0);
		}

		if (argc >= SERVINFO_2_OFFSET + 3 + 1) {
			s = get_server_info(&argv[SERVINFO_2_OFFSET]);
		}

		gem::Client c(s.address, s.client_port, s.peer_port);
		gem::Value v;

		try {
			gem::json j = gem::json::parse(argv[COMMAND_ARG_2]);
			v = gem::value_from_json(j);
		} catch (gem::json::parse_error &e) {
			std::cerr << "Error occured while trying to parse '"
				          << argv[COMMAND_ARG_2] << "': " << e.what() << "\n";
				exit(1);
		}

		std::cout << "Setting '" << argv[COMMAND_ARG_1] << "' to '"
		          << argv[COMMAND_ARG_2] << "'..." << std::endl;

		try {
			c.set_value(argv[COMMAND_ARG_1], v);
		} catch (std::exception &e) {
			std::cerr << "Error: " << e.what() << "\n";
			exit(1);
		}

		std::cout << "Done." << "\n";
		exit(0);
	}

	if (strcmp(argv[1], "get") == 0) {
		if (argc < COMMAND_ARG_1 + 1) {
			std::cerr << "Error: get requires at least 1 argument." << std::endl;
			exit(0);
		}

		if (argc >= SERVINFO_1_OFFSET + 3 + 1) {
			s = get_server_info(&argv[SERVINFO_1_OFFSET]);
		}

		gem::Client c(s.address, s.peer_port, s.client_port);

		std::cout << "Getting '" << argv[COMMAND_ARG_1] <<"'..." << std::endl;

		try {
			gem::QueryResult v = c.get_value(argv[COMMAND_ARG_1]);
			std::cout << "Value is: " << v[0].to_json_value() << std::endl;
		} catch (std::exception &e) {
			std::cerr << "Error: " << e.what() << "\n";
			exit(1);
		}

		std::cout << "Done." << "\n";
		exit(0);
	}

	if (strcmp(argv[1], "configinfo") == 0) {
		if (argc >= SERVINFO_0_OFFSET + 3 + 1) {
			s = get_server_info(&argv[SERVINFO_0_OFFSET]);
		}

		gem::Client c(s.address, s.client_port, s.peer_port);

		std::cout << "Getting config..." << std::endl;

		try {
			gem::Config v = c.peer_get_config();
			std::cout << "Config is: " << gem::json(v) << std::endl;
		} catch (std::exception &e) {
			std::cerr << "Error: " << e.what() << "\n";
			exit(1);
		}

		std::cout << "Done." << "\n";
		exit(0);
	}

	/*if (strcmp(argv[1], "syncinfo") == 0) {
		if (argc >= SERVINFO_0_OFFSET + 3 + 1) {
			s = get_server_info(&argv[SERVINFO_1_OFFSET]);
		}

		gem::Client c(s.address, s.client_port, s.peer_port);

		std::cout << "Getting config..." << std::endl;

		try {
			gem::SyncStatusData v = c.peer_get_sync_status_data();
			std::cout << "Sync Status Info is: " << gem::json(v) << std::endl;
		} catch (std::exception &e) {
			std::cerr << "Error: " << e.what() << "\n";
			exit(1);
		}

		std::cout << "Done." << "\n";
		exit(0);
	}*/

	if (strcmp(argv[1], "dump") == 0) {
		if (argc >= SERVINFO_0_OFFSET + 3) {
			s = get_server_info(&argv[SERVINFO_0_OFFSET]);
		}

		gem::Client c(s.address, s.client_port, s.peer_port);

		std::cout << "Getting dump..." << std::endl;

		try {
			std::cout << c.dump().dump(4) << std::endl;
		} catch (std::exception &e) {
			std::cerr << "Error: " << e.what() << "\n";
			exit(1);
		}

		std::cout << "Done." << "\n";
		exit(0);
	}


	std::cout << "Unknown Command: " << argv[1] << std::endl;
	print_help_message(argv[0]);
	exit(1);

	return 0;
}