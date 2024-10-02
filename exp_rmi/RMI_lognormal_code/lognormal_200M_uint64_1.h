#include <cstddef>
#include <cstdint>
namespace lognormal_200M_uint64_1 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 201326608;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "lognormal_200M_uint64_1";
uint64_t lookup(uint64_t key, size_t* err);
}
