#include "common.h"

class BufferPool {
  private:
    uint64_t num_frames;

  public:
    BufferPool(uint64_t num_frames_); 
    ~BufferPool(); 

    void readPage(page_id_t pageID); 
    void writePage(page_id_t pageID); 

    void flushAllPage(page_id_t pageID);
    void flushAllPages();
};
