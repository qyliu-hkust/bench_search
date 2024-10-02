#include <cstddef>
#include <cstdint>
namespace wiki_ts_200M_uint64_5 {
bool load(char const* dataPath);
void cleanup();
const size_t RMI_SIZE = 6291472;
const uint64_t BUILD_TIME_NS = 0;
const char NAME[] = "wiki_ts_200M_uint64_5";
uint64_t lookup(uint64_t key, size_t* err);
}
