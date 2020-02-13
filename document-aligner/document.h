#pragma once
#include <stdint.h>
#include <cstdint>

namespace bitextor {

struct WordScore {
  uint32_t hash; // Yes there will be collisions.  Some document pairs might accidentally be suggested.
  float tfidf;
};

inline float DocumentDot(const WordScore *first, const WordScore *first_end, const WordScore *second, const WordScore *second_end) {
  float ret = 0.0;
  while (first != first_end && second != second_end) {
    if (*first < *second) {
      ++first;
    } else if (*first > *second) {
      ++second;
    } else {
      ret += first->tfidf * second->tfidf;
      ++first;
      ++second;
    }
  }
  return ret;
}

} // namespace bitextor
