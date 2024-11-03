#include <map>
#include <stdint.h>
#include <string>
#include "common.hpp"
#include "json.hpp"

namespace gem {

/**
 * @brief      Describes a type presented in a store value.
 */
enum class Type {
	None   = 0,
	Int    = 1,
	Float  = 2,
	String = 3,
	Array  = 4
};

/**
 * @brief      The values that are present in a store
 */
union ValueStorage {
	int64_t i;
	long double f;
	void *v;
	// How will you handle non scalar data types?
	// - will you define shortarrays in this union?
	//   eg: `char[sizeof(int64_t)] c4;` ?
};

struct Value {
	Type type = Type::None;
	ValueStorage value;
};

struct Store {

	using UserKey = String;
	using HashKey = int64_t;
	using ValueMap = Map<HashKey, Value>;

	using Status = int;

	/* GLOBAL_WRITE_LOCK? */
	/* Would it be possible to do simultaneous writes? */

	ValueMap v;

/*	HashKey convert_to_hash(const UserKey &u) {
		// ...
	}

	Value get(UserKey h) {

	}

	Value get_pattern(UserKey h) {

	}

	Value set(UserKey h) {

	}

	Value del(UserKey h) {

	}

	Value del_pattern(UserKey h) {

	}

	Status bulk_update(BulkStore) {

	}*/
};

struct Query {
	enum QueryType {
		NORMAL,
		PATTERN
	};

	QueryType query_type;
	String query_string_json;
};

struct QueryResult {
	Store::ValueMap values;
};

};