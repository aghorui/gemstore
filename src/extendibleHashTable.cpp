#include "extendibleHashTable.hpp"
#include "xxHash.hpp"
#include <iostream>

namespace xxhash {

const uint64_t ExtendibleHashTable::SEED = 0x123456789ABCDEF;

// Value class implementation
Value::Value() : type(ValueType::INT), vecInt(nullptr) {}

Value::~Value() {
    clearVectors();
}

// Copy constructor
Value::Value(const Value& other) : type(other.type), stringValue(other.stringValue) {
    copyValueFrom(other);
}

// Assignment operator
Value& Value::operator=(const Value& other) {
    if (this != &other) {
        clearVectors();
        type = other.type;
        stringValue = other.stringValue;
        copyValueFrom(other);
    }
    return *this;
}

void Value::setValue(ValueType t, bool value) {
    clearVectors();
    type = t;
    boolValue = value;
}

void Value::setValue(ValueType t, int value) {
    clearVectors();
    type = t;
    intValue = value;
}

void Value::setValue(ValueType t, float value) {
    clearVectors();
    type = t;
    floatValue = value;
}

void Value::setValue(ValueType t, const std::string &value) {
    clearVectors();
    type = t;
    stringValue = value;
}

void Value::setVectorInt(const std::vector<int>& vec) {
    clearVectors();
    type = ValueType::VEC_INT;
    vecInt = new std::vector<int>(vec);
}

void Value::setVectorString(const std::vector<std::string>& vec) {
    clearVectors();
    type = ValueType::VEC_STRING;
    vecString = new std::vector<std::string>(vec);
}

void Value::setVectorFloat(const std::vector<float>& vec) {
    clearVectors();
    type = ValueType::VEC_FLOAT;
    vecFloat = new std::vector<float>(vec);
}

void Value::setVectorBool(const std::vector<bool>& vec) {
    clearVectors();
    type = ValueType::VEC_BOOL;
    vecBool = new std::vector<bool>(vec);
}

void Value::setVectorValue(const std::vector<Value>& vec) {
    clearVectors();
    type = ValueType::VEC_VALUE;
    vecValue = new std::vector<Value>(vec);
}

void Value::clearVectors() {
    switch(type) {
        case ValueType::VEC_INT:
            delete vecInt;
            break;
        case ValueType::VEC_STRING:
            delete vecString;
            break;
        case ValueType::VEC_FLOAT:
            delete vecFloat;
            break;
        case ValueType::VEC_BOOL:
            delete vecBool;
            break;
        case ValueType::VEC_VALUE:
            delete vecValue;
            break;
        default:
            break;
    }
}

void Value::copyValueFrom(const Value& other) {
    switch(other.type) {
        case ValueType::BOOL:
            boolValue = other.boolValue;
            break;
        case ValueType::INT:
            intValue = other.intValue;
            break;
        case ValueType::FLOAT:
            floatValue = other.floatValue;
            break;
        case ValueType::VEC_INT:
            vecInt = new std::vector<int>(*other.vecInt);
            break;
        case ValueType::VEC_STRING:
            vecString = new std::vector<std::string>(*other.vecString);
            break;
        case ValueType::VEC_FLOAT:
            vecFloat = new std::vector<float>(*other.vecFloat);
            break;
        case ValueType::VEC_BOOL:
            vecBool = new std::vector<bool>(*other.vecBool);
            break;
        case ValueType::VEC_VALUE:
            vecValue = new std::vector<Value>(*other.vecValue);
            break;
        default:
            break;
    }
}

// Bucket class implementation
Bucket::Bucket(size_t depth) : localDepth(depth) {
    items.reserve(BUCKET_SIZE);
}

bool Bucket::isFull() const {
    return items.size() >= BUCKET_SIZE;
}

bool Bucket::insert(const std::string& key, const Value& value) {
    if (isFull()) return false;
    items.emplace_back(key, value);
    return true;
}

// ExtendibleHashTable class implementation
ExtendibleHashTable::ExtendibleHashTable() : globalDepth(0) {
    directory.push_back(std::make_shared<Bucket>(0));
}

size_t ExtendibleHashTable::getDirectoryIndex(const std::string& key) const {
    uint64_t hash = XXHash64::hash(
        reinterpret_cast<const unsigned char*>(key.c_str()),
        key.length(),
        SEED
    );
    return hash & ((1 << globalDepth) - 1);
}

void ExtendibleHashTable::split(size_t bucketIndex) {
    auto oldBucket = directory[bucketIndex];

    if (oldBucket->localDepth == globalDepth) {
        // Double the directory size
        directory.resize(directory.size() * 2);
        for (size_t i = 0; i < directory.size() / 2; ++i) {
            directory[directory.size() / 2 + i] = directory[i];
        }
        ++globalDepth;
    }

    // Create new bucket
    auto newBucket = std::make_shared<Bucket>(oldBucket->localDepth + 1);
    oldBucket->localDepth = newBucket->localDepth;

    // Redistribute items
    std::vector<std::pair<std::string, Value>> oldItems;
    oldItems.swap(oldBucket->items);

    size_t mask = 1 << (oldBucket->localDepth - 1);
    size_t highBucketIndex = bucketIndex | mask;

    // Update directory pointers
    for (size_t i = 0; i < directory.size(); ++i) {
        if ((i & ((1 << oldBucket->localDepth) - 1)) == (bucketIndex & ((1 << oldBucket->localDepth) - 1))) {
            if (i & mask) {
                directory[i] = newBucket;
            } else {
                directory[i] = oldBucket;
            }
        }
    }

    // Reinsert items
    for (const auto& item : oldItems) {
        insert(item.first, item.second);
    }
}

void ExtendibleHashTable::insert(const std::string& key, const Value& value) {
    size_t index = getDirectoryIndex(key);
    auto bucket = directory[index];

    // Check if key already exists
    for (auto& item : bucket->items) {
        if (item.first == key) {
            item.second = value;
            return;
        }
    }

    // If bucket is not full, insert directly
    if (!bucket->isFull()) {
        bucket->insert(key, value);
        return;
    }

    // If bucket is full, split and retry insertion
    split(index);
    insert(key, value);
}

bool ExtendibleHashTable::get(const std::string& key, Value& value) const {
    size_t index = getDirectoryIndex(key);
    const auto& bucket = directory[index];

    for (const auto& item : bucket->items) {
        if (item.first == key) {
            value = item.second;
            return true;
        }
    }
    return false;
}

bool ExtendibleHashTable::remove(const std::string& key) {
    size_t index = getDirectoryIndex(key);
    auto& bucket = directory[index];

    for (auto it = bucket->items.begin(); it != bucket->items.end(); ++it) {
        if (it->first == key) {
            bucket->items.erase(it);
            return true;
        }
    }
    return false;
}

size_t ExtendibleHashTable::size() const {
    std::unordered_set<std::shared_ptr<Bucket>> uniqueBuckets;
    size_t totalItems = 0;

    for (const auto& bucket : directory) {
        if (uniqueBuckets.insert(bucket).second) {
            totalItems += bucket->items.size();
        }
    }
    return totalItems;
}

size_t ExtendibleHashTable::getGlobalDepth() const {
    return globalDepth;
}

void ExtendibleHashTable::print() const {
    std::unordered_set<std::shared_ptr<Bucket>> printedBuckets;

    std::cout << "Global Depth: " << globalDepth << std::endl;
    for (size_t i = 0; i < directory.size(); ++i) {
        if (printedBuckets.insert(directory[i]).second) {
            std::cout << "Directory Index " << i << " (Local Depth: "
                     << directory[i]->localDepth << "):" << std::endl;
            for (const auto& item : directory[i]->items) {
                std::cout << "  " << item.first << std::endl;
            }
        }
    }
}

}
