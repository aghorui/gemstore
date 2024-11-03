#ifndef COMMON_HPP
#define COMMON_HPP

#include "json.hpp"
#include <map>
#include <queue>
#include <string>
#include <iostream>

namespace gem {

using json = nlohmann::json;

using String = std::string;

template<typename T>
using Queue = std::queue<T>;

template <typename K, typename V>
using Map = std::map<K, V>;

bool validate_json(String json_str);

};

#endif