#include "common.hpp"
#include "networking.hpp"
#include <cassert>
#include <fstream>
#include <csignal>

gem::Server *server_ptr = nullptr;

void terminate_handler(int) {
	std::cerr << "Stopping Server...\n";
	assert(server_ptr != nullptr && "Server ptr should not be null.");
	server_ptr->close();
	std::cerr << "Exiting.\n";
	exit(0);
}

int main(int argc, char **argv) {
	gem::Config cfg;
	gem::Store store;

	if (argc >= 2) {
		if (strcmp("-h", argv[1]) == 0 ||
		    strcmp("--help", argv[1]) == 0) {
			std::cerr << GEMSTORE_NAME << " version " << GEMSTORE_VERSION << "\n";
			std::cerr << "\n";
			std::cerr
				<< "Gemstore is a distributed key value datastore.\n"
				<< "The following command-line options are available:\n\n"
				<< "  " << argv[0] << " [-h|--help]           Print this help message\n"
				<< "  " << argv[0] << " [-v|--version]        Print version and exit\n"
				<< "  " << argv[0] << " --dump-config [path]  Dumps the default config to the path, or stdout if unspecified.\n"
				<< "  " << argv[0] << " <config file path>    Read config file before starting program\n"
				<< "\n";
			exit(0);

		} else if (strcmp("-v", argv[1]) == 0 ||
		           strcmp("--version", argv[1]) == 0) {
			std::cerr << GEMSTORE_NAME << " version " << GEMSTORE_VERSION << "\n";
			std::cerr << "\n";
			exit(0);

		} else if (strcmp("--dump-config", argv[1]) == 0) {
			if (argc >= 3) {
				std::ofstream file(argv[2]);
				if (!file.good()) {
					std::cerr << "Error: Couldn't open specified destination file: "
					          << argv[1] << "\n";
					exit(1);
				}

				gem::json j;
				gem::to_json(j, cfg);

				file << j;

				exit(0);

			} else {
				gem::json j;
				gem::to_json(j, cfg);

				std::cout << j.dump(4);

				exit(0);
			}

		} else {
			std::ifstream file(argv[1]);

			if (!file.good()) {
				std::cerr << "Error: Couldn't open specified configuration file: "
				          << argv[1] << "\n";
				exit(1);
			}

			try {
				gem::json j = gem::json::parse(file);
				cfg = j.template get<gem::Config>();
			} catch (std::exception &e) {
				std::cerr << "Error occured while trying to parse '"
				          << argv[1] << "': " << e.what() << "\n";
				exit(1);
			}
		}
	}

	gem::Server server(cfg, store, {});
	server_ptr = &server;

	std::cerr << GEMSTORE_NAME << " version " << GEMSTORE_VERSION << "\n";
	std::cerr << "\n";

	struct sigaction terminate_action;
	memset(&terminate_action, 0, sizeof(terminate_action));
	terminate_action.sa_handler = terminate_handler;
	sigaction(SIGINT, &terminate_action, NULL);

	server.start();

	return 0;
}