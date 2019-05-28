
#include "search.h"
#include "align.h"
#include "utils/common.h"

#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <utility>
#include <memory>

#include <boost/make_unique.hpp>
#include <boost/functional/hash.hpp>


namespace search {


    Dynamic::Dynamic(size_t r, size_t c) {
      // set matrix dimensions with an extra column and an extra row
      rows = r + 1;
      cols = c + 1;

      // initialise
      scores = boost::make_unique<double[]>(rows * cols);
      back_pointers = boost::make_unique<char[]>(rows * cols);

      std::fill(scores.get(), scores.get() + rows * cols, 0);
      std::fill(back_pointers.get(), back_pointers.get() + rows * cols, '.');
    }

    double *Dynamic::get_score(size_t r, size_t c) {
      if ((r > rows - 1) || (c > cols - 1))
        throw std::runtime_error("invalid cost scores access");

      return &scores[r * cols + c];
    }

    char *Dynamic::get_backpointer(size_t r, size_t c) {
      if ((r > rows - 1) || (c > cols - 1))
        throw std::runtime_error("invalid cost back_pointers access");

      return &back_pointers[r * cols + c];
    }

    void Dynamic::process(std::vector<utils::scoremap> &smap_list) {
      if(smap_list.size() != rows - 1) {
        throw std::runtime_error("Dimensions in Dynamic::process do not match!");
      }

      for (size_t s = 0; s < smap_list.size(); ++s) {
        utils::scoremap::reverse_iterator it = smap_list.at(s).rbegin();
        while (it != smap_list.at(s).rend()) {
          alignments.insert({{s, it->second.first}, it->first});
          ++it;
        }
      }

      double score, best_score;
      char pointer;

      for (size_t r = 0; r < smap_list.size(); ++r) {
        for (size_t c = 0; c < cols - 1; ++c) {
          best_score = *get_score(r, c + 1);
          pointer = '^';

          score = *get_score(r + 1, c);
          if (score > best_score) {
            best_score = score;
            pointer = '<';
          }

          boost::unordered_map<utils::sizet_pair, double>::const_iterator got = alignments.find({r, c});
          if (got != alignments.end()) {
            score = got->second + *get_score(r, c);

            if (score > best_score) {
              best_score = score;
              pointer = 'm';
            }
          }

          *get_score(r + 1, c + 1) = best_score;
          *get_backpointer(r, c) = pointer;

        }
      }
    }

    void Dynamic::show() {
      std::cout << rows << "x" << cols << "\n";
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          std::cout << *(get_score(r, c)) << "\t";
        }
        std::cout << "\n";
      }
      std::cout << "\n------" << std::endl;

      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          std::cout << *(get_backpointer(r, c)) << "\t";
        }
        std::cout << "\n";
      }

    }

    void Dynamic::extract_matches(utils::matches_vec &res) {
      res.clear();
      int i = rows - 2;
      int j = cols - 2;
      char pointer;

      while (i >= 0 && j >= 0) {
        pointer = *get_backpointer(i, j);
        if (pointer == '^') {
          i -= 1;
        } else if (pointer == '<') {
          j -= 1;
        } else if (pointer == 'm') {

          double score;
          boost::unordered_map<utils::sizet_pair, double>::const_iterator got = alignments.find({i, j});
          if (got != alignments.end())
            score = got->second;
          else
            score = 0;

          res.push_back(utils::match(i, i, j, j, score));
          i -= 1;
          j -= 1;
        } else {
          throw std::runtime_error("Unexpected value in Dynamic::extract_matches!");
        }
      }

      std::reverse(res.begin(), res.end());

    }

    Munkres::Munkres(size_t r, size_t c, bool _min_cost) : min_cost(_min_cost) {
      // The algorithm expects more columns than rows in the cost matrix.
      if (r > c) {
        rows = c;
        cols = r;
        transposed = true;
      } else {
        rows = r;
        cols = c;
        transposed = false;
      }

      // initialise
      costs = boost::make_unique<double[]>(rows * cols);
      mask = boost::make_unique<int[]>(rows * cols);
      row_cover = boost::make_unique<bool[]>(rows);
      col_cover = boost::make_unique<bool[]>(cols);
      path0 = boost::make_unique<int[]>(2 * rows * cols + 2);
      path1 = boost::make_unique<int[]>(2 * rows * cols + 2);

      // default values
      if (min_cost)
        std::fill(costs.get(), costs.get() + rows * cols, 0);
      else
        std::fill(costs.get(), costs.get() + rows * cols, search::max_score);
      std::fill(mask.get(), mask.get() + rows * cols, 0);
      std::fill(row_cover.get(), row_cover.get() + rows, false);
      std::fill(col_cover.get(), col_cover.get() + cols, false);
      std::fill(path0.get(), path0.get() + (2 * rows * cols + 2), 0);
      std::fill(path1.get(), path1.get() + (2 * rows * cols + 2), 0);

    }

    void Munkres::process(std::vector<utils::scoremap> &smap_list) {
      double val;
      for (size_t i = 0; i < smap_list.size(); ++i) {
        utils::scoremap::reverse_iterator it = smap_list.at(i).rbegin();
        while (it != smap_list.at(i).rend()) {
          if (min_cost)
            val = it->first;
          else
            val = search::max_score - it->first;

          if (transposed) {
            *get_cost(it->second.first, i) = val;
            ++it;
          } else {
            *get_cost(i, it->second.first) = val;
            ++it;
          }
        }
      }

      run_step();

    }

    void Munkres::process(const std::vector<double> &input_costs) {
      if (input_costs.size() != rows * cols) {
        throw std::runtime_error("dimensions mismatch");
      }

      double val;
      for (size_t i = 0; i < input_costs.size(); ++i) {
        if (min_cost)
          val = input_costs.at(i);
        else
          val = search::max_score - input_costs.at(i);

        if (transposed)
          costs[(i % rows) * cols + i / rows] = val;
        else
          costs[i] = val;
      }

      run_step();

    }

    void Munkres::show() {
      std::cout << rows << "x" << cols << "\n";
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          std::cout << *(get_cost(r, c)) << "\t";
        }
        std::cout << "\n";
      }
      std::cout << "\n------" << std::endl;


      std::cout << "\t";
      for (size_t c = 0; c < cols; ++c) {
        std::cout << "\t" << col_cover[c];
      }
      std::cout << "\n\n";

      for (size_t r = 0; r < rows; ++r) {
        std::cout << row_cover[r] << " \t\t";
        for (size_t c = 0; c < cols; ++c) {
          std::cout << *(get_mask(r, c)) << "\t";
        }
        std::cout << "\n";
      }

    }

    double *Munkres::get_cost(size_t r, size_t c) {
      if ((r > rows - 1) || (c > cols - 1))
        throw std::runtime_error("invalid cost matrix access");

      return &costs[r * cols + c];
    }

    int *Munkres::get_mask(size_t r, size_t c) {
      if ((r > rows - 1) || (c > cols - 1))
        throw std::runtime_error("invalid mask matrix access");

      return &mask[r * cols + c];
    }

    void Munkres::run_step(short step) {
      std::pair<size_t, size_t> path_data(0, 0);
      size_t col_count;
      bool found;

      switch (step) {
        case 0:
        case 1:
          step1();
        case 2:
          step2();
        case 3:
          col_count = step3();
          if (col_count >= cols || col_count >= rows) {
            break;
          }
        case 4:
          found = step4(path_data);
          if (!found) {
            run_step(6);
            break;
          }
        case 5:
          step5(path_data);
          run_step(3);
          break;
        case 6:
          step6();
          run_step(4);
          break;

        default:
          throw std::runtime_error("invalid state");

      }

    }

    void Munkres::step1() {
      double min_val;
      size_t c;
      for (size_t r = 0; r < rows; ++r) {
        min_val = *get_cost(r, 0);
        for (c = 0; c < cols; ++c) {
          if (*get_cost(r, c) < min_val) {
            min_val = *get_cost(r, c);
          }
        }

        for (c = 0; c < cols; ++c) {
          *get_cost(r, c) -= min_val;
        }
      }
    }

    void Munkres::step2() {
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (*get_cost(r, c) == 0 && !row_cover[r] && !col_cover[c]) {
            *get_mask(r, c) = 1;
            row_cover[r] = 1;
            col_cover[c] = 1;
          }
        }
      }

      std::fill(row_cover.get(), row_cover.get() + rows, false);
      std::fill(col_cover.get(), col_cover.get() + cols, false);
    }

    size_t Munkres::step3() {
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (*get_mask(r, c) == 1) {
            col_cover[c] = true;
          }
        }
      }

      size_t col_count = 0;
      for (size_t c = 0; c < cols; ++c) {
        if (col_cover[c]) {
          ++col_count;
        }
      }

      return col_count;
    }

    bool Munkres::step4(std::pair<size_t, size_t> &ref) {
      std::pair<size_t, size_t> zero(0, 0);
      bool found;

      while (true) {
        found = find_zero(zero);
        if (!found) {
          return false;
        }

        *get_mask(zero.first, zero.second) = 2;

        size_t found_col = find_star_in_row(zero.first);
        if (found_col == cols) {
          ref.first = zero.first;
          ref.second = zero.second;
          return true;
        }

        row_cover[zero.first] = 1;
        col_cover[found_col] = 0;
      }

    }

    bool Munkres::find_zero(std::pair<size_t, size_t> &ref) {
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (*get_cost(r, c) == 0 && !row_cover[r] && !col_cover[c]) {
            ref.first = r;
            ref.second = c;
            return true;
          }
        }
      }

      return false;
    }

    size_t Munkres::find_prime_in_row(size_t r) {
      for (int c = cols - 1; c >= 0; --c) {
        if (*get_mask(r, c) == 2) {
          return c;
        }
      }

      return cols;
    }

    size_t Munkres::find_star_in_row(size_t r) {
      for (int c = cols - 1; c >= 0; --c) {
        if (*get_mask(r, c) == 1) {
          return c;
        }
      }

      return cols;
    }

    size_t Munkres::find_star_in_col(size_t c) {
      for (int r = rows - 1; r >= 0; --r) {
        if (*get_mask(r, c) == 1) {
          return r;
        }
      }

      return rows;
    }

    void Munkres::step5(std::pair<size_t, size_t> path_data) {
      size_t path_cout = 1;
      path0[path_cout - 1] = path_data.first;
      path1[path_cout - 1] = path_data.second;

      while (true) {
        size_t found_row = find_star_in_col(path1[path_cout - 1]);
        if (found_row == rows)
          break;

        ++path_cout;
        path0[path_cout - 1] = found_row;
        path1[path_cout - 1] = path1[path_cout - 2];

        size_t found_col = find_prime_in_row(path0[path_cout - 1]);
        ++path_cout;
        path0[path_cout - 1] = path0[path_cout - 2];

        if (found_col == cols)
          path1[path_cout - 1] = -1;
        else
          path1[path_cout - 1] = found_col;
      }

      augment_path(path_cout);

      std::fill(row_cover.get(), row_cover.get() + rows, false);
      std::fill(col_cover.get(), col_cover.get() + cols, false);

      erase_primes();

    }

    void Munkres::augment_path(size_t path_cout) {
      for (size_t p = 0; p < path_cout; ++p) {
        if (*get_mask(path0[p], path1[p]) == 1) {
          *get_mask(path0[p], path1[p]) = 0;
        } else {
          *get_mask(path0[p], path1[p]) = 1;
        }
      }
    }

    void Munkres::erase_primes() {
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (*get_mask(r, c) == 2) {
            *get_mask(r, c) = 0;
          }
        }
      }
    }

    void Munkres::step6() {
      // find min value
      double minval = std::numeric_limits<double>::max();
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (row_cover[r] == 0 && col_cover[c] == 0) {
            minval = std::min<double>(minval, *get_cost(r, c));
          }
        }
      }

      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (row_cover[r] == 1)
            *get_cost(r, c) += minval;
          if (col_cover[c] == 0)
            *get_cost(r, c) -= minval;
        }
      }
    }

    void Munkres::extract_matches(utils::matches_vec &res) {
      res.clear();

      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
          if (*get_mask(r, c) == 1) {
            if (transposed){
              res.push_back(utils::match(c, c, r, r, *get_cost(r,c)));
            }
            else{
              res.push_back(utils::match(r, r, c, c, *get_cost(r,c)));
            }
            break;
          }
        }
      }
    }

    void FindMatches(utils::matches_vec &matches, std::vector<utils::scoremap> &scorelist,
                     size_t translated_size, size_t english_size, double threshold) {
      Dynamic finder(translated_size, english_size);
      finder.process(scorelist);
      finder.extract_matches(matches);
      FilterMatches(matches, scorelist, threshold);
    }

    void FilterMatches(utils::matches_vec &matches, std::vector<utils::scoremap> &scorelist, double threshold) {
      for (auto m: matches) {
        if (!m.first.same() || !m.second.same())
          throw "Inconsistent data: Only 1:1 alignments can be filtered!";
      }

      utils::matches_vec matches_cpy(matches);
      matches.clear();

      utils::scoremap::reverse_iterator it;
      for (auto m: matches_cpy) {
        it = scorelist.at(m.first.from).rbegin();
        while (it != scorelist.at(m.first.from).rend()) {
          if (it->second.first == m.second.from && it->first > threshold) {
            matches.push_back(m);
            break;
          }

          ++it;
        }
      }
    }


}
