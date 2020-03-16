#include "murmur_hash.h"

namespace bitextor {

uint64_t MurmurHashCombine(uint64_t k, uint64_t seed) {
  const uint64_t m = 0xc6a4a7935bd1e995ULL;
  const int r = 47;
 
  uint64_t h = seed ^ (8 * m);
 
  k *= m;
  k ^= k >> r;
  k *= m;
 
  h ^= k;
  h *= m;
 
  h ^= h >> r;
  h *= m;
  h ^= h >> r;
 
  return h;
}

}