#ifndef COMMON_HPP
#define COMMON_HPP

#include "json.hpp"
#include "httplib.h"
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

template <typename T>
using Set = std::set<T>;

template <typename T>
using Vector = std::vector<T>;

bool validate_json(String json_str);

};

#endif