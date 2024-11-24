#ifndef EXTENDIBLEHASHTABLE_H

#define EXTENDIBLEHASHTABLE_H

#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include "common.hpp"
#include "json.hpp"


namespace xxhash {

// Enum for value types
enum class ValueType {
    BOOL,
    INT,
    FLOAT,
    STRING,
    VEC_INT,
    VEC_STRING,
    VEC_FLOAT,
    VEC_BOOL,
    VEC_VALUE
};

class Value;
class Value {
public:
    ValueType type;
    union {
        bool boolValue;
        int intValue;
        float floatValue;
        std::vector<int>* vecInt;
        std::vector<std::string>* vecString;
        std::vector<float>* vecFloat;
        std::vector<bool>* vecBool;
        std::vector<Value>* vecValue;
    };
    std::string stringValue;

    Value();
    ~Value();

    // Copy constructor
    Value(const Value& other);

    // Assignment operator
    Value& operator=(const Value& other);

    void setValue(ValueType t, bool value);
    void setValue(ValueType t, int value);
    void setValue(ValueType t, float value);
    void setValue(ValueType t, const std::string &value);

    bool getValueAsBool() const {
        if (type != ValueType::BOOL) {
            throw std::runtime_error("Value type is not BOOL");
        }
        return boolValue;
    }

    int getValueAsInt() const {
        if (type != ValueType::INT) {
            throw std::runtime_error("Value type is not INT");
        }
        return intValue;
    }

    float getValueAsFloat() const {
        if (type != ValueType::FLOAT) {
            throw std::runtime_error("Value type is not FLOAT");
        }
        return floatValue;
    }

    std::string getValueAsString() const {
        if (type != ValueType::STRING) {
            throw std::runtime_error("Value type is not STRING");
        }
        return stringValue;
    }

    void setVectorInt(const std::vector<int>& vec);
    void setVectorString(const std::vector<std::string>& vec);
    void setVectorFloat(const std::vector<float>& vec);
    void setVectorBool(const std::vector<bool>& vec);
    void setVectorValue(const std::vector<Value>& vec);

    std::vector<int> getVectorInt() const {
        if (type != ValueType::VEC_INT) {
            throw std::runtime_error("Value type is not VEC_INT");
        }
        return *vecInt;
    }

    std::vector<std::string> getVectorString() const {
        if (type != ValueType::VEC_STRING) {
            throw std::runtime_error("Value type is not VEC_STRING");
        }
        return *vecString;
    }

    std::vector<float> getVectorFloat() const {
        if (type != ValueType::VEC_FLOAT) {
            throw std::runtime_error("Value type is not VEC_FLOAT");
        }
        return *vecFloat;
    }

    std::vector<bool> getVectorBool() const {
        if (type != ValueType::VEC_BOOL) {
            throw std::runtime_error("Value type is not VEC_BOOL");
        }
        return *vecBool;
    }

    std::vector<Value> getVectorValue() const {
        if (type != ValueType::VEC_VALUE) {
            throw std::runtime_error("Value type is not VEC_VALUE");
        }
        return *vecValue;
    }

private:
    void clearVectors();
    void copyValueFrom(const Value& other);
};




static inline void to_json(gem::json& j, const Value& p) {
	switch (p.type) {
	   case ValueType::BOOL:  j = gem::json{nullptr};
				break;
        case ValueType::INT: j = gem::json{p.intValue};
            break;
        case ValueType::FLOAT: j = gem::json{p.floatValue};
            break;
        case ValueType::STRING: j = gem::json{p.stringValue.c_str()};
            break;
        case ValueType::VEC_VALUE: {
    		j = gem::json::array();
            gem::json tmp;
            std::vector<Value> store = p.getVectorValue();
    		for (auto &item : store) {
                to_json(tmp, item);
    			j.push_back(tmp);
    		}
    		break;
	       }
        default: throw std::invalid_argument("Invalid value type");
    	}

    j = gem::json{
		//{ "value", p.value.storage }
		{ "value", gem::json(nullptr)}
	};
}

static inline void from_json(const gem::json& j, xxhash::Value& p) {
    switch (j.type()) {
        case gem::json::value_t::boolean:
            p.setValue(xxhash::ValueType::BOOL, j.get<bool>());
            break;
        case gem::json::value_t::number_integer:
            p.setValue(xxhash::ValueType::INT, j.get<int>());
            break;
        case gem::json::value_t::number_float:
            p.setValue(xxhash::ValueType::FLOAT, j.get<float>());
            break;
        case gem::json::value_t::string:
            p.setValue(xxhash::ValueType::STRING, j.get<std::string>());
            break;
        case gem::json::value_t::array: {
            xxhash::Value tmp;
            std::vector<xxhash::Value> vec;
      		p.type = xxhash::ValueType::VEC_VALUE;
    		std::vector<xxhash::Value> *arr = new std::vector<xxhash::Value>();
    		for (auto value : j) {
                xxhash::Value *tmp = new xxhash::Value();
    			from_json(value, *tmp);
    			arr->push_back(*tmp);
    		}
            p.setVectorValue(*arr);
            break;
        }
        default: throw std::invalid_argument("Invalid value type: " + j.dump());
}
}


// Class to represent a bucket
class Bucket {
public:
    static const size_t BUCKET_SIZE = 4;
    std::vector<std::pair<std::string, xxhash::Value >> items;
    size_t localDepth;

    Bucket(size_t depth = 0);

    bool isFull() const;
    bool insert(const std::string& key, const xxhash::Value& value);
};

// Class to represent an extendible hash table
class ExtendibleHashTable {
private:
    std::vector<std::shared_ptr<Bucket>> directory;
    size_t globalDepth;
    static const uint64_t SEED;

    size_t getDirectoryIndex(const std::string& key) const;
    void split(size_t bucketIndex);

public:
    ExtendibleHashTable();

    void insert(const std::string& key, const xxhash::Value& value);
    bool get(const std::string& key, xxhash::Value& value) const;
    bool remove(const std::string& key);
    size_t size() const;
    size_t getGlobalDepth() const;
    void print() const;
};

};

#endif // EXTENDIBLEHASHTABLE_H
