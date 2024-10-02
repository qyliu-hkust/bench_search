#include <cstddef>
#include <cstdint>
namespace osm_cellids_800M_uint64_8 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 24592;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "osm_cellids_800M_uint64_8";
uint64_t lookup(uint64_t key, size_t* err);
}
