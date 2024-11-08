#include "store.hpp"
#include <functional>

namespace gem {

#ifdef DUMMY_IMPLEMENTATION

ValueType get_type_from_json(const json &j) {
	if      (j.is_number_integer()) return ValueType::Int;
	else if (j.is_number_float())   return ValueType::Float;
	else if (j.is_string())         return ValueType::String;
	else if (j.is_array())          return ValueType::Array;
	else if (j.is_null())           return ValueType::None;
	else assert(0 && "Invlaid value type");
}

bool Store::contains(const Store::Key &u) {
	return vmap.contains(u);
}

Value Store::get(const Store::Key &u) {
	if (vmap.count(u) < 1) {
		throw KeyNotFoundException(u);
	}

	Value v;
	v.type = get_type_from_json(vmap[u]),
	v.storage = vmap[u];

	return v;
}

Value Store::get_pattern(const Store::Key &) {
	assert(0 && "not implemented");
}

bool Store::set(const Store::Key &u, Value &v) {
	vmap[u] = v.storage;
	return true;
}

bool Store::del(const Store::Key &u) {
	if (vmap.count(u) < 1) {
		return false;
	} else {
		vmap.erase(u);
		return true;
	}
}

bool Store::del_pattern(const Store::Key &) {
	assert(0 && "not implemented");
}

bool Store::bulk_update(Vector<KeyValuePair> &kvs) {
	for (auto &kv : kvs) {
		set(kv.key, kv.value);
	}

	return true;
}

json Store::dump() {
	return vmap;
}

Value value_from_json(const json &j) {
	Value v;
	v.type = get_type_from_json(j);
	v.storage = j;
	return v;
}

#else

// TODO

#endif

};