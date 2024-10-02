#include "normal_200M_uint64_6.h"
#include "normal_200M_uint64_6_data.h"
#include <math.h>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <iostream>
namespace normal_200M_uint64_6 {
char* L1_PARAMETERS = nullptr; 
bool load(char const* dataPath) {
  {
    std::ifstream infile(std::filesystem::path(dataPath) / "normal_200M_uint64_6_L1_PARAMETERS", std::ios::in | std::ios::binary);
    if (!infile.good()) return false;
    L1_PARAMETERS = (char*) malloc(5120);
    if (L1_PARAMETERS == NULL) return false;
    infile.read((char*)L1_PARAMETERS, 5120);
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

inline double linear(double alpha, double beta, double inp) {
    return std::fma(beta, inp, alpha);
}

inline size_t FCLAMP(double inp, double bound) {
  if (inp < 0.0) return 0;
  return (inp > bound ? bound : (size_t)inp);
}

uint64_t lookup(uint64_t key, size_t* err) {
  double fpred;
  size_t modelIndex;
  fpred = linear(L0_PARAMETER0, L0_PARAMETER1, (double)key);
  modelIndex = FCLAMP(fpred, 128.0 - 1.0);
  fpred = cubic(*((double*) (L1_PARAMETERS + (modelIndex * 40) + 0)), *((double*) (L1_PARAMETERS + (modelIndex * 40) + 8)), *((double*) (L1_PARAMETERS + (modelIndex * 40) + 16)), *((double*) (L1_PARAMETERS + (modelIndex * 40) + 24)), (double)key);
  *err = *((uint64_t*) (L1_PARAMETERS + (modelIndex * 40) + 32));

  return FCLAMP(fpred, 200000000.0 - 1.0);
}
} // namespace
