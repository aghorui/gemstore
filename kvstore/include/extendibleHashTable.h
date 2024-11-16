#include <iostream>
#include "page/extendibleHashTableHeaderPage.h"
#include "page/extendibleHashTableDirectoryPage.h"
#include "page/extendibleHashTableBucketPage.h"
 
class ExtendibleHashTable {
  public:
    ExtendibleHashTable();
    ~ExtendibleHashTable();
     
    int insert(std::string key, std::string value); 

    std::string getValue(std::string key);

    int remove(std::string key);
};
