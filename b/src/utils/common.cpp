
#include "common.h"


#include <iostream>
#include <vector>
#include <regex>
#include <ctype.h>


#include <boost/functional.hpp>


namespace utils {

    std::string PieceToString(StringPiece sp) {
      return std::string(sp.data(), static_cast<std::string::size_type>(sp.size()));
    }

    void SplitStringPiece(std::vector<StringPiece> &vec, StringPiece sp, char c, size_t pos, size_t max_split) {
      vec.clear();

      if (sp.size() <= 0 || pos >= unsigned(sp.size()))
        return;

      size_t num_split = 0;
      size_t res = std::find(sp.data() + pos, sp.data() + sp.length(), c) - sp.data();
      while (res < unsigned(sp.data() + sp.length() - sp.data())) {
        if (max_split and num_split >= max_split) {
          res = sp.length();
          break;
        }

        vec.push_back(sp.substr(pos, res - pos));

        pos = res + 1;
        res = std::find(sp.data() + pos, sp.data() + sp.length(), c) - sp.data();
        num_split += 1;
      }

      vec.push_back(sp.substr(pos, res - pos));

    }

    void SplitStringPiece(std::vector<StringPiece> &vec, StringPiece sp, int (*check_func)(int), size_t pos,
                          size_t max_split) {
      vec.clear();

      if (sp.size() <= 0 || pos >= unsigned(sp.size()))
        return;

      size_t num_split = 0;
      size_t res = std::find_if(sp.data() + pos, sp.data() + sp.length(), boost::ptr_fun(isspace)) - sp.data();
      while (res < unsigned(sp.data() + sp.length() - sp.data())) {
        if (max_split and num_split >= max_split) {
          res = sp.length();
          break;
        }

        vec.push_back(sp.substr(pos, res - pos));

        pos = res + 1;
        res = std::find_if(sp.data() + pos, sp.data() + sp.length(), boost::ptr_fun(*check_func)) - sp.data();
        num_split += 1;
      }

      vec.push_back(sp.substr(pos, res - pos));

    }


} // namespace utils
