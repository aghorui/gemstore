#include "networking.hpp"

int main() {
	gem::Store store;
	gem::Server server(store, {});
	server.start();

	return 0;
}