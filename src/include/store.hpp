/*
 * store.hpp - Storage Infrastructure.
 */

#ifndef GEM_STORE_HPP
#define GEM_STORE_HPP

#include <exception>
#include <map>
#include <mutex>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include "common.hpp"
#include "json.hpp"

namespace gem {
/**
 * @brief      Describes a type presented in a store value.
 */
enum class ValueType {
	None   = 0,
	Bool   = 1,
	Int    = 2,
	Float  = 3,
	String = 4,
	Array  = 5
};


#ifdef DUMMY_IMPLEMENTATION

using ValueStorage = json;

struct Value {
	ValueType type = ValueType::None;
	ValueStorage storage;

	json to_json_value() const {
		return storage;
	}
};

ValueType get_type_from_json(const json &j);
Value value_from_json(const json &j);

#else

/**
 * @brief      The values that are present in a store
 */
union ValueStorage {
	int64_t i;
	long double f;
	void *data;
	// How will you handle non scalar data types?
	// - will you define shortarrays in this union?
	//   eg: `char[sizeof(int64_t)] c4;` ?
};

struct Value {
	ValueType type = ValueType::None;
	ValueStorage storage;

	json to_json_value() {
		// TODO
	}
};


ValueType get_type_from_json(const json &j);
Value value_from_json(const json &j);

#endif

/*
static inline void to_json(json& j, const Value& v) {
	switch (v.type) {
	case Type::None:   j = json{nullptr};                               break;
	case Type::Int:    j = json{v.storage.i};                           break;
	case Type::Float:  j = json{v.storage.f};                           break;
	case Type::String: j = json{(char *) v.storage.data};               break;

	case Type::Array:
		j = json::array();
		std::vector<Value> *source = (std::vector<Value> *) v.storage.data;
		for (auto &item : *source) {
			j.push_back(item);
		}
		break;
	}
}

static inline void from_json(const json& j, Value& v) {
	if (j.is_null()) {
		v.type = Type::None;
	} else if (j.is_number_integer()) {
		v.type = Type::Int;
		v.storage.i = j.template get<int64_t>();
	} else if (j.is_string()) {
		v.type = Type::String;
		v.storage.data = j.template get<std::string>();
	} else if (j.is_number_float()) {
		v.type = Type::Float;
		v.storage.f = j.template get<long double>();
	} else if (j.is_array()) {
		v.type = Type::Array;
		std::vector<Value> *arr = new std::vector<Value>();
		for (auto value : j) {
			json new_value;

			from_json(j, v);
			arr->push_back(new_value)
		}
		v.storage.data = arr;
	}
}*/

enum class MergeAttributes {
	NUM_NONE,
	NUM_MIN,
	NUM_MAX,
	NUM_AVERAGE,
	STR_NONE,
	STR_CONCAT,
	ARR_NONE,
	ARR_CONCAT
};

using Key = String;

struct KeyValuePair {
	Key key;
	Value value;
};

static inline void to_json(json& j, const KeyValuePair& p) {
	j = json{
		{ "key", p.key },
		{ "value", p.value.storage }
	};
}

static inline void from_json(const json& j, KeyValuePair& p) {
	j.at("key").get_to(p.key);
	p.value.type = get_type_from_json(j.at("value"));
	j.at("value").get_to(p.value.storage);
}

struct Store {

#ifdef DUMMY_IMPLEMENTATION
	using ValueMap = json;
#else
	// TODO
#endif

	using Status = int;

	/* GLOBAL_WRITE_LOCK? */
	/* Would it be possible to do simultaneous writes? */
	std::mutex vmap_lock;
	ValueMap vmap;

	bool contains(const Key &u);
	Value get(const Key &u);
	Value get_pattern(const Key &u);
	bool set(const Key &u, Value &v);
	bool del(const Key &u);
	bool del_pattern(const Key &u);

	// template<typename T>
	// json bulk_get(T begin, T end);
	Vector<KeyValuePair> bulk_get(Vector<Key> &k);
	bool bulk_update(Vector<KeyValuePair> &k);
	json dump(); // dump everything as a json object

	struct KeyNotFoundException : public std::invalid_argument {
		const Key u;
		KeyNotFoundException(const Key u): std::invalid_argument(u) {}
	};
};

};

#endif