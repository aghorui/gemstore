#include <fstream>
#include "common.h"

class DiskManager {
  private:
    std::fstream fs; 
    uint64_t fileLength;

  public:
    DiskManager(const std::string filename);

    ~DiskManager();
     
    int increaseDiskSize(); 

    int readPage(page_id_t pageID, char *pageData);

    int writePage(page_id_t pageID, char *pageData);
};

