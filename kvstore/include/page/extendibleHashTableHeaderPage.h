#include "common.h"

static constexpr uint64_t HEADER_PAGE_METADATA_SIZE = sizeof(uint32_t);
static constexpr uint64_t HEADER_MAX_DEPTH = 9;
static constexpr uint64_t HEADER_ARRAY_SIZE = 1 << HEADER_MAX_DEPTH;

class ExtendibleHashTableHeaderPage {
  private:
    page_id_t directoryPageIDs[HEADER_ARRAY_SIZE];
    uint32_t maxDepth;
  public:
    ExtendibleHashTableHeaderPage(int32_t max_depth = HEADER_MAX_DEPTH);
    ~ExtendibleHashTableHeaderPage();

    uint32_t hashToDirectoryPageID(uint32_t hash);
    page_id_t getDirectoryPage(uint32_t hash);
    int setDirectoryPage(uint32_t hash, page_id_t directory_page_id);
}; 

