#include "common.h"

static constexpr uint64_t DIRECTORY_PAGE_METADATA_SIZE = sizeof(uint32_t);
static constexpr uint64_t DIRECTORY_MAX_DEPTH = 9;
static constexpr uint64_t DIRECTORY_ARRAY_SIZE = 1 << DIRECTORY_MAX_DEPTH;

class ExtendibleHashTableDirectoryPage {
  private:
    uint32_t maxDepth;
    uint32_t globalDepth;
    uint32_t localDepth;
    page_id_t bucketPageIDs[DIRECTORY_ARRAY_SIZE];
  public:
    ExtendibleHashTableDirectoryPage(int32_t max_depth = DIRECTORY_MAX_DEPTH);
    ~ExtendibleHashTableDirectoryPage();

    uint32_t hashToBucketPageID(uint32_t hash);
    page_id_t getBucketPage(uint32_t hash);
    int setBucketPage(uint32_t hash, page_id_t directory_page_id);

    int incLocalDepth();
    int decLocalDepth();

    int incGlocalDepth();
    int decGlocalDepth();

    // int incBucketPageSize(uint32_t hash, page_id_t directory_page_id);
    // int decBucketPageSize(uint32_t hash, page_id_t directory_page_id);
}; 


