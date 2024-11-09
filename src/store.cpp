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

bool Store::contains(const Key &u) {
	vmap_lock.lock();
	bool ret = vmap.contains(u);
	vmap_lock.unlock();
	return ret;
}

Value Store::get(const Key &u) {
	vmap_lock.lock();
	if (vmap.count(u) < 1) {
		throw KeyNotFoundException(u);
	}

	Value v;
	v.type = get_type_from_json(vmap[u]),
	v.storage = vmap[u];
	vmap_lock.unlock();
	return v;
}

Value Store::get_pattern(const Key &) {
	assert(0 && "not implemented");
}

bool Store::set(const Key &u, Value &v) {
	vmap_lock.lock();
	vmap[u] = v.storage;
	vmap_lock.unlock();
	return true;
}

bool Store::del(const Key &u) {
	vmap_lock.lock();
	if (vmap.count(u) < 1) {
		vmap_lock.unlock();
		return false;
	} else {
		vmap.erase(u);
		vmap_lock.unlock();
		return true;
	}
}

bool Store::del_pattern(const Key &) {
	assert(0 && "not implemented");
}

Vector<KeyValuePair> Store::bulk_get(Vector<Key> &k) {
	Vector<KeyValuePair> kvs;

	for (auto &key : k) {
		KeyValuePair kv;
		kv.key = key;
		kv.value = get(key);
		kvs.push_back(kv);
	}

	return kvs;
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