#include <cstddef>
#include <cstdint>
namespace osm_cellids_800M_uint64_2 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 100663328;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "osm_cellids_800M_uint64_2";
uint64_t lookup(uint64_t key, size_t* err);
}
