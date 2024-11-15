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

bool Store::set(const Key &u, const Value &v) {
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
		merge_and_set(kv.key, kv.value);
	}

	return true;
}


bool Store::set_merge_attributes(Vector<MergeAttributeEntry> attrs) {
	for (auto &attr : attrs) {
		merge_attributes[attr.key] = attr.attr;
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

#define CHECK_TYPE(x, m, t) if ((x).type != (t)) { \
	log() << "Type mismatch between merge attribute (" << merge_attr_to_str.at(m) \
	      <<  ") and local value: " << x.storage; \
	return false; \
}

#define CHECK_TYPE2(x, m, t1, t2) if (((x).type != (t1)) && ((x).type != (t2))) { \
	log() << "Type mismatch between merge attribute (" << merge_attr_to_str.at(m) \
	      << ") and local value: " << x.storage; \
	return false; \
}

bool Store::merge_and_set(const Key &u, const Value &v) {
	auto attr = merge_attributes.find(u);

	if (attr == merge_attributes.end()) {
		set(u, v);
	}

	if (!contains(u)) {
		set(u, v);
	}

	Value v_local = get(u);

	if (v_local.type != v.type) {
		log() << "Type mismatch between remote ('" << v.storage << "')"
		      << " and local ('" << v_local.storage << "') " << "Ignoring";
		return false;
	}

	switch (attr->second) {
	case NUM_NONE:
		CHECK_TYPE2(v, attr->second, ValueType::Int, ValueType::Float);
		set(u, v);
		break;

	case NUM_MIN:
		CHECK_TYPE2(v, attr->second, ValueType::Int, ValueType::Float);
		if (v.storage < v_local.storage) {
			set(u, v);
		}
		break;

	case NUM_MAX:
		CHECK_TYPE2(v, attr->second, ValueType::Int, ValueType::Float);
		if (v.storage > v_local.storage) {
			set(u, v);
		}
		break;

	case NUM_AVERAGE:
		CHECK_TYPE2(v, attr->second, ValueType::Int, ValueType::Float);
		{ // Need to put this in scope, otherwise we get jump bypasses
		  // variable initialization
			float avg_val =
				v.storage.template get<float>() +
				v_local.storage.template get<float>();
			avg_val = avg_val / 2;
			Value set_v;
			set_v.type = ValueType::Float;
			set_v.storage = json(avg_val);
			set(u, set_v);
		}
		break;

	case STR_NONE:
		CHECK_TYPE(v, attr->second, ValueType::String);
		set(u, v);
		break;

	case STR_CONCAT:
		CHECK_TYPE(v, attr->second, ValueType::String);
		{ // Need to put this in scope, otherwise we get jump bypasses
		  // variable initialization
			std::string concat_val =
				v_local.storage.template get<std::string>() +
				v.storage.template get<std::string>();
			Value set_v;
			set_v.type = ValueType::String;
			set_v.storage = json(concat_val);
			set(u, set_v);
		}
		break;

	case ARR_NONE:
		CHECK_TYPE(v, attr->second, ValueType::Array);
		set(u, v);
		break;

	case ARR_CONCAT:
		CHECK_TYPE(v, attr->second, ValueType::Array);
		{ // Need to put this in scope, otherwise we get jump bypasses
		  // variable initialization
			std::string concat_val =
				v.storage.template get<std::string>() +
				v_local.storage.template get<std::string>();
			Value set_v;
			set_v.type = ValueType::String;
			set_v.storage = json(concat_val);
			set(u, set_v);
		}
		break;

	case ARR_UNION:
		CHECK_TYPE(v, attr->second, ValueType::Array);
		{
			Set<json> union_set(v.storage.begin(), v.storage.end());
			union_set.insert(v_local.storage.begin(), v_local.storage.end());
			Value set_v;
			set_v.type = ValueType::Array;
			set_v.storage = json(union_set);
			set(u, set_v);
		}
		break;
	}
	return true;
}



#else

// TODO

#endif

};