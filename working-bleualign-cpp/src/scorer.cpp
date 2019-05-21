
#include "scorer.h"
#include "ngram.h"
#include "utils/common.h"
#include "util/string_piece.hh"

#include <iostream>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>


namespace scorer {

    void Tokenize(std::vector<std::string> &token_vec, const std::string &text) {
      token_vec.clear();

      boost::sregex_iterator it(text.begin(), text.end(), tokenize_regex);
      boost::sregex_iterator end;
      std::stringstream ss;
      size_t last_pos = 0;

      for (; it != end; ++it) {
        std::string token = text.substr(last_pos, it->position() - last_pos);

        if (token.length() > 0) {
          token_vec.push_back(token);
        }
        if (it->str() != " ") {
          token_vec.push_back(it->str());
        }

        last_pos = it->position() + it->str().length();
      }

      if (last_pos < text.length()) {
        token_vec.push_back(text.substr(last_pos, text.length() - last_pos));
      }

    }

    void normalize(std::vector<std::string> &token_vec, const std::string &text, const std::string &language_type) {
      std::string normalized_text = scorer::ApplyNormalizeRules(text, scorer::normalize1_rules);
      normalized_text = scorer::ApplyNormalizeRules(normalized_text, scorer::normalize2_rules);

      if (language_type == "western") {
        boost::algorithm::to_lower(normalized_text);
        normalized_text.insert(0, " ");
        normalized_text.push_back(' ');
      }

      scorer::Tokenize(token_vec, normalized_text);
    }

    void
    cook_ref_set(std::vector<ngram::NGramCounter> &counter_vec, const std::string &text, unsigned short ngram_size) {
      std::vector<std::string> token_vec;
      scorer::normalize(token_vec, text, "western");

      ngram::NGramCounter ngc(ngram_size);
      ngc.process(token_vec);

      counter_vec.push_back(ngc);

    }


}