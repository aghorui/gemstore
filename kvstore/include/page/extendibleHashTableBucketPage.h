#include "common.h"
#include <string>

static constexpr uint64_t BUCKET_PAGE_METADATA_SIZE = sizeof(uint32_t);
static constexpr uint64_t BUCKET_MAX_DEPTH = 9;
static constexpr uint64_t BUCKET_ARRAY_SIZE = 1 << BUCKET_MAX_DEPTH;

class ExtendibleHashTableBucketPage {
  private:
    uint64_t bucketPageIDs[BUCKET_ARRAY_SIZE];
    uint32_t maxSize;
    uint32_t size;
  public:
    ExtendibleHashTableBucketPage(int32_t max_size = BUCKET_ARRAY_SIZE);
    ~ExtendibleHashTableBucketPage();

    void insert();
    void search();
    void remove();

}; 

