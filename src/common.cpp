#include "common.hpp"

namespace gem {

bool validate_json(String json_str) {
	try {
		auto result = json::parse(json_str);
	} catch (json::parse_error &e) {
		std::cerr << "Could not parse json: " << e.what() << std::endl;
		return false;
	}

	return true;
}

};
