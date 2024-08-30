//
//  rmi.h
//  bench_search
//
//  Created by Liu Qiyu on 2024/8/27.
//

#ifndef rmi_h
#define rmi_h

#include <math.h>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fb_200m_uint64_rmi {
const double L0_PARAMETER0 = 4.56560426804208;
const double L0_PARAMETER1 = 0.00000042313015733017296;
char* L1_PARAMETERS;
const size_t RMI_SIZE = 786448;
const uint64_t BUILD_TIME_NS = 3787558000;
const char NAME[] = "fb_200m_uint64_rmi";

bool load(char const* dataPath) {
  {
    std::ifstream infile(std::filesystem::path(dataPath) / "fb_200m_uint64_rmi_L1_PARAMETERS", std::ios::in | std::ios::binary);
    if (!infile.good()) return false;
    L1_PARAMETERS = (char*) malloc(786432);
    if (L1_PARAMETERS == NULL) return false;
    infile.read((char*)L1_PARAMETERS, 786432);
    if (!infile.good()) return false;
  }
  return true;
}
void cleanup() {
    free(L1_PARAMETERS);
}

inline double linear(double alpha, double beta, double inp) {
    return std::fma(beta, inp, alpha);
}

inline size_t FCLAMP(double inp, double bound) {
  if (inp < 0.0) return 0;
  return (inp > bound ? bound : (size_t)inp);
}

uint64_t lookup(uint64_t key, size_t* err) {
  size_t modelIndex;
  double fpred;
  fpred = linear(L0_PARAMETER0, L0_PARAMETER1, (double)key);
  modelIndex = FCLAMP(fpred, 32768.0 - 1.0);
  fpred = linear(*((double*) (L1_PARAMETERS + (modelIndex * 24) + 0)), *((double*) (L1_PARAMETERS + (modelIndex * 24) + 8)), (double)key);
  *err = *((uint64_t*) (L1_PARAMETERS + (modelIndex * 24) + 16));

  return FCLAMP(fpred, 200000000.0 - 1.0);
}

} // namespace



#endif /* rmi_h */
