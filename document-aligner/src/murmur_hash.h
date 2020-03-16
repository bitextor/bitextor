#include "util/murmur_hash.hh"

namespace bitextor {

uint64_t MurmurHashCombine(uint64_t k, uint64_t seed);

const auto MurmurHashNative = util::MurmurHashNative;

}