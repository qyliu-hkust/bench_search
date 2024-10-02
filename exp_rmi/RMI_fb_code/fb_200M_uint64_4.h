#include <cstddef>
#include <cstdint>
namespace fb_200M_uint64_4 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 12582928;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "fb_200M_uint64_4";
uint64_t lookup(uint64_t key, size_t* err);
}
