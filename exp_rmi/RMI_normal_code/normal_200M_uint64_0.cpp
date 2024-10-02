#include "normal_200M_uint64_0.h"
#include "normal_200M_uint64_0_data.h"
#include <math.h>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <iostream>
namespace normal_200M_uint64_0 {
char* L1_PARAMETERS = nullptr; 
bool load(char const* dataPath) {
  {
    std::ifstream infile(std::filesystem::path(dataPath) / "normal_200M_uint64_0_L1_PARAMETERS", std::ios::in | std::ios::binary);
    if (!infile.good()) return false;
    L1_PARAMETERS = (char*) malloc(1310720);
    if (L1_PARAMETERS == NULL) return false;
    infile.read((char*)L1_PARAMETERS, 1310720);
    if (!infile.good()) return false;
  }
  return true;
}
void cleanup() {
    free(L1_PARAMETERS);
}

inline double cubic(double a, double b, double c, double d, double x) {
    auto v1 = std::fma(a, x, b);
    auto v2 = std::fma(v1, x, c);
    auto v3 = std::fma(v2, x, d);
    return v3;
}

inline size_t FCLAMP(double inp, double bound) {
  if (inp < 0.0) return 0;
  return (inp > bound ? bound : (size_t)inp);
}

uint64_t lookup(uint64_t key, size_t* err) {
  size_t modelIndex;
  double fpred;
  fpred = cubic(L0_PARAMETER0, L0_PARAMETER1, L0_PARAMETER2, L0_PARAMETER3, (double)key);
  modelIndex = (uint64_t) fpred;
  fpred = cubic(*((double*) (L1_PARAMETERS + (modelIndex * 40) + 0)), *((double*) (L1_PARAMETERS + (modelIndex * 40) + 8)), *((double*) (L1_PARAMETERS + (modelIndex * 40) + 16)), *((double*) (L1_PARAMETERS + (modelIndex * 40) + 24)), (double)key);
  *err = *((uint64_t*) (L1_PARAMETERS + (modelIndex * 40) + 32));

  return FCLAMP(fpred, 200000000.0 - 1.0);
}
} // namespace
