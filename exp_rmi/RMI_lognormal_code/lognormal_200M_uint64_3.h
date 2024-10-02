#include <cstddef>
#include <cstdint>
namespace lognormal_200M_uint64_3 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 20971536;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "lognormal_200M_uint64_3";
uint64_t lookup(uint64_t key, size_t* err);
}
