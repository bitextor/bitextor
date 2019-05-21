
#ifndef FAST_BLEUALIGN_SCORER_H
#define FAST_BLEUALIGN_SCORER_H

#include "ngram.h"

#include <string>
#include <regex>
#include <boost/regex.hpp>


namespace scorer {

    typedef std::pair<std::regex, std::string> rule_pair;

    const static boost::regex tokenize_regex(
            "([\\{-\\~\\[-\\` -\\&\\(-\\+\\:-\\@\\/])|(?:(?<![0-9])([\\.,]))|(?:([\\.,])(?![0-9]))|(?:(?<=[0-9])(-))");

    const static rule_pair normalize1_rules[] = {
            std::make_pair(std::regex("<skipped>"), ""),  // strip "skipped" tags
            std::make_pair(std::regex("-\\n"), ""),  // strip end-of-line hyphenation and join lines
            std::make_pair(std::regex("\\n"), " "),  // join lines
    };

    const static rule_pair normalize2_rules[] = {
            std::make_pair(std::regex("&amp;"), "&"),
            std::make_pair(std::regex("&lt;"), "<"),
            std::make_pair(std::regex("&gt;"), ">"),
            std::make_pair(std::regex("&quot;"), "\""),
    };

    template<size_t N>
    std::string ApplyNormalizeRules(std::string s, const rule_pair (&rules)[N]) {
      for (size_t i = 0; i < N; ++i) {
        s = std::regex_replace(s, rules[i].first, rules[i].second);
      }

      return s;
    }

    void Tokenize(std::vector<std::string> &token_vec, const std::string &text);

    void normalize(std::vector<std::string> &token_vec, const std::string &text, const std::string &language_type);

    void
    cook_ref_set(std::vector<ngram::NGramCounter> &counter_vec, const std::string &text, unsigned short ngram_size);

}

#endif //FAST_BLEUALIGN_SCORER_H
