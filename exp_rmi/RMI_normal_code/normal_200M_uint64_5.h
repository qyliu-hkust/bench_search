#include <cstddef>
#include <cstdint>
namespace normal_200M_uint64_5 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 5152;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "normal_200M_uint64_5";
uint64_t lookup(uint64_t key, size_t* err);
}
