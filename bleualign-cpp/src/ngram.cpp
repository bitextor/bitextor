
#include "ngram.h"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <memory>

#include <boost/make_unique.hpp>
#include <boost/functional/hash.hpp>


namespace ngram {

    NGramCounter::NGramCounter(unsigned short n) : ngram_size(n) {
      data.assign(n, ngram_map());
    }

    int NGramCounter::get(const NGramContainer &key) {
      size_t idx = get_map_index(key);
      ngram_map::iterator it;
      it = data.at(idx).find(key);

      if (it != data.at(idx).end()) {
        return it->second;
      } else {
        return 0;
      }
    }

    void NGramCounter::increment(const NGramContainer &ng) {
      size_t map_idx = get_map_index(ng);
      ngram_map::iterator it;
      it = data.at(map_idx).find(ng);

      if (it != data.at(map_idx).end()) {
        // FOUND
        it->second = it->second + 1;
      } else {
        // NOT FOUND
        data.at(map_idx).insert(std::make_pair(ng, 1));
      }

      ++total_freq;
    }

    void NGramCounter::process(std::vector<std::string> &tokens) {
      // set-up iterators
      std::unique_ptr<std::vector<std::string>::iterator[]> token_iterators(
              boost::make_unique<std::vector<std::string>::iterator[]>(ngram_size));
      for (unsigned short i = 0; i < ngram_size; ++i) {
        token_iterators[i] = tokens.begin();
        std::advance(token_iterators[i], i);
      }

      // base
      for (unsigned short i = 0; i < ngram_size - 1; ++i) {
        increment_helper(token_iterators, i + 1);
      }

      // continue
      while (token_iterators[ngram_size - 1] != tokens.end()) {
        increment_helper(token_iterators, ngram_size);

        for (unsigned short i = 0; i < ngram_size; ++i) {
          ++token_iterators[i];
        }
      }

      tokens_processed += tokens.size();

    }

    void NGramCounter::increment_helper(std::unique_ptr<std::vector<std::string>::iterator[]> &token_iterators,
                                        unsigned short ngram) {
      switch (ngram) {
        case 4:
          increment(NGramContainer(token_iterators[ngram - 4]->c_str(), token_iterators[ngram - 3]->c_str(),
                                   token_iterators[ngram - 2]->c_str(), token_iterators[ngram - 1]->c_str()));
        case 3:
          increment(NGramContainer(token_iterators[ngram - 3]->c_str(), token_iterators[ngram - 2]->c_str(),
                                   token_iterators[ngram - 1]->c_str()));
        case 2:
          increment(NGramContainer(token_iterators[ngram - 2]->c_str(), token_iterators[ngram - 1]->c_str()));
        case 1:
          increment(NGramContainer(token_iterators[ngram - 1]->c_str()));
          break;
      }
    }

    size_t NGramCounter::count_tokens() {
      size_t num = 0;
      for (auto s: data) {
        num += s.size();
      }
      return num;
    }

    size_t NGramCounter::get_map_index(const NGramContainer &key) {
      return key.data()->size() - 1;
    }

    bool operator==(const NGramContainer &a, const NGramContainer &b) {
      if (a.data()->size() != b.data()->size()) return false;

      for (size_t i = 0; i < a.data()->size(); ++i) {
        if (a.data()->at(i) != b.data()->at(i)) return false;
      }

      return true;
    }

    std::size_t hash_value(const NGramContainer &e) {
      std::size_t seed = 0;

      for (size_t i = 0; i < e.data()->size(); ++i) {
        boost::hash_combine(seed, e.data()->at(i));
      }

      return seed;
    }

}
