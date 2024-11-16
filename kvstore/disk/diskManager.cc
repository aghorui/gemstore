#include "disk/diskManager.h"

DiskManager::DiskManager(const std::string filename) {
  fs.open(filename);
};

DiskManager::~DiskManager() {
  fs.close();
};

