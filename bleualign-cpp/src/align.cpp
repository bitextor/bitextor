
#include "align.h"
#include "scorer.h"
#include "ngram.h"
#include "search.h"
#include "util/string_piece.hh"
#include "utils/common.h"
#include "utils/logging.h"

#include <algorithm>
#include <math.h>
#include <boost/make_unique.hpp>
#include <vector>
#include <memory>

#include "utils/CompressedWriter.h"


namespace align {

    void AlignDocuments(const std::string &output_path, const utils::AlignData &align_data, const StringPiece matched_text1_uri,
                        StringPiece matched_text2_uri, double threshold) {
      //LOG_INFO << "Processing " << matched_text1_uri << " and " << matched_text2_uri;

      utils::matches_vec matches;

      Align(matches, align_data.umap_text1translated.at(matched_text1_uri.data()),
            align_data.umap_text2.at(matched_text2_uri.data()), threshold);
      WriteAlignedTextToFile(output_path, matches, align_data.umap_text1.at(matched_text1_uri.data()),
                             align_data.umap_text2.at(matched_text2_uri.data()));

    }

    void Align(utils::matches_vec &matches, const std::vector<std::string> &text1translated_doc,
               const std::vector<std::string> &text2_doc, double threshold) {

      std::vector<utils::scoremap> scorelist;
      EvalSents(scorelist, text1translated_doc, text2_doc, 2, 3);
      search::FindMatches(matches, scorelist, text1translated_doc.size(), text2_doc.size(), threshold);
      GapFiller(matches, text1translated_doc, text2_doc, 3, threshold);

    }

    /* given list of test sentences and list of reference sentences, calculate bleu scores */
    void EvalSents(std::vector<utils::scoremap> &scorelist, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, unsigned short ngram_size, size_t maxalternatives) {

      std::vector<ngram::NGramCounter> cooktarget;
      for (size_t i = 0; i < text2_doc.size(); ++i) {
        scorer::cook_ref_set(cooktarget, text2_doc.at(i), ngram_size);
      }

      for (size_t trans_i = 0; trans_i < text1translated_doc.size(); ++trans_i) {
        // copied over from bleu.py to minimize redundancy
        std::vector<std::string> test_normalized;
        scorer::normalize(test_normalized, text1translated_doc.at(trans_i), "western");

        std::vector<size_t> guess;
        for (unsigned short k = 0; k < ngram_size; ++k) {
          guess.push_back(std::max<unsigned short>(test_normalized.size() - k, 0));
        }

        ngram::NGramCounter test_counts(ngram_size);
        test_counts.process(test_normalized);

        utils::scoremap smap;
        for (size_t c = 0; c < cooktarget.size(); ++c) {

          float logbleu = 0.0;
          std::vector<int> cooked_test_correct;
          cooked_test_correct.assign(ngram_size, 0);

          ngram::ngram_map::iterator map_it;
          for (size_t order = 1; order <= ngram_size; ++order) {
            map_it = test_counts.begin(order);
            while (map_it != test_counts.end(order)) {
              int cooktarget_count = cooktarget.at(c).get(map_it->first);
              if (cooktarget_count > 0) {
                // update correct counts
                int test_count = test_counts.get(map_it->first);
                cooked_test_correct.at(order - 1) += std::min(cooktarget_count, test_count);
              }

              ++map_it;
            }

            // logbleu
            logbleu += log(cooked_test_correct.at(order - 1)) - log(guess.at(order - 1));
          }

          logbleu /= ngram_size;
          logbleu += std::min<float>(0, 1 - static_cast<float>(cooktarget.at(c).processed()) /
                                            static_cast<float>(test_normalized.size()));
          float score = exp(logbleu);

          if (score > 0) {
            // calculate bleu score in reverse direction
            logbleu = 0.0;
            for (size_t order = 1; order <= ngram_size; ++order) {
              logbleu += log(cooked_test_correct.at(order - 1)) -
                         log(std::max<int>(int(cooktarget.at(c).processed()) - order + 1, 0));
            }
            logbleu /= ngram_size;
            logbleu += std::min<float>(0, 1 - static_cast<float>(test_normalized.size()) /
                                              static_cast<float>(cooktarget.at(c).processed()));
            float score2 = exp(logbleu);
            float meanscore = (2 * score * score2) / (score + score2);

            smap.insert(utils::scoremap::value_type(meanscore, std::make_pair(c, cooked_test_correct)));
          }
        }

        // keep top N items
        if (smap.size() > maxalternatives) {
          utils::scoremap::iterator rem_it = smap.end();
          std::advance(rem_it, -(maxalternatives));
          smap.erase(smap.begin(), rem_it);
        }

        scorelist.push_back(smap);
      }
    }

    void GapFiller(utils::matches_vec &matched, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, size_t gap_limit, double threshold) {

      // check that matches vector contains only 1:1 matches
      for (auto m: matched) {
        if (!m.first.same() || !m.second.same())
          throw "Inconsistent data in matches!";
      }

      std::unique_ptr<int[]> matches_arr_translated = boost::make_unique<int[]>(text1translated_doc.size());
      std::unique_ptr<int[]> matches_arr_text2 = boost::make_unique<int[]>(text2_doc.size());
      std::fill(matches_arr_translated.get(), matches_arr_translated.get() + text1translated_doc.size(), -1);
      std::fill(matches_arr_text2.get(), matches_arr_text2.get() + text2_doc.size(), -1);


      for (auto m: matched) {
        matches_arr_translated[m.first.from] = m.second.from;
        matches_arr_text2[m.second.from] = m.first.from;
      }

      std::vector<std::string> merged_text_translated;
      utils::vec_pair merged_pos_translated;
      std::vector<std::string> merged_text_text2;
      utils::vec_pair merged_pos_text2;

      for (auto &m: matched) {
        for (int post = 0; post < 2; ++post) {

          if (post == 0) { // pre
            PreGapMergedSentences(merged_text_translated, merged_pos_translated, text1translated_doc,
                                  matches_arr_translated, m.first.from, gap_limit);
            PreGapMergedSentences(merged_text_text2, merged_pos_text2, text2_doc,
                                  matches_arr_text2, m.second.from, gap_limit);
          } else if (post == 1) { // post
            PostGapMergedSentences(merged_text_translated, merged_pos_translated, text1translated_doc,
                                   matches_arr_translated, text1translated_doc.size(), m.first.from, gap_limit);
            PostGapMergedSentences(merged_text_text2, merged_pos_text2, text2_doc,
                                   matches_arr_text2, text2_doc.size(), m.second.from, gap_limit);
          }

          if (merged_text_translated.size() == 1 && merged_text_text2.size() == 1)
            continue;

          std::vector<utils::scoremap> scorelist;
          EvalSents(scorelist, merged_text_translated, merged_text_text2, 2, 3);

          // find max
          float max_val = -1;
          size_t max_pos_translate;
          size_t max_pos_text2;
          for (size_t i = 0; i < scorelist.size(); ++i) {
            if (scorelist.at(i).size() == 0) {
              continue;
            }

            if (scorelist.at(i).rbegin()->first > max_val && scorelist.at(i).rbegin()->first > threshold) {
              max_val = scorelist.at(i).rbegin()->first;
              max_pos_translate = i;
              max_pos_text2 = scorelist.at(i).rbegin()->second.first;
            }
          }

          // update match
          if (max_val != -1) {
            m = utils::match(
                    merged_pos_translated.at(max_pos_translate).first,
                    merged_pos_translated.at(max_pos_translate).second,
                    merged_pos_text2.at(max_pos_text2).first,
                    merged_pos_text2.at(max_pos_text2).second, max_val);
          }


          FillMatches(matches_arr_translated, matches_arr_text2, m);

        }
      }


    }

    void PreGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                               const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr, size_t pos,
                               size_t gap_limit) {

      int start_post = pos - 1;
      while (start_post >= 0) {
        if (matches_arr[start_post] != -1) break;
        if (start_post < signed(pos) - signed(gap_limit) + 1) break;
        --start_post;
      }

      ProduceMergedSentences(merged_text, merged_pos, docs, start_post + 1, pos, gap_limit, true);

    }


    void PostGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr,
                                size_t matches_arr_size, size_t pos, size_t gap_limit) {

      int start_post = pos + 1;
      while (start_post < signed(matches_arr_size)) {
        if (matches_arr[start_post] != -1) break;
        if (start_post > signed(pos) + signed(gap_limit) - 1) break;
        ++start_post;
      }

      ProduceMergedSentences(merged_text, merged_pos, docs, pos, start_post - 1, gap_limit, false);

    }


    void ProduceMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, size_t from, size_t to, size_t limit,
                                bool reverse) {
      merged_text.clear();
      merged_pos.clear();
      std::stringstream ss;

      if (reverse) {
        size_t limited_end = std::min(limit, to - from + 1);
        for (size_t i = 0; i < limited_end; ++i) {
          ss.str("");
          for (size_t j = 0; j <= i; ++j) {
            ss << docs.at(to - i + j) << " ";
          }

          merged_text.push_back(ss.str());
          merged_pos.push_back(std::make_pair(to - i, to));
        }

      } else {
        size_t limited_end = std::min(limit, to - from + 1);
        for (size_t i = 0; i < limited_end; ++i) {
          ss.str("");
          for (size_t j = 0; j <= i; ++j) {
            ss << docs.at(from + j) << " ";
          }

          merged_text.push_back(ss.str());
          merged_pos.push_back(std::make_pair(from, from + i));
        }

      }

    }


    void FillMatches(std::unique_ptr<int[]> &arr1, std::unique_ptr<int[]> &arr2, utils::match m) {
      for (size_t i = m.first.from; i <= m.first.to; ++i) {
        arr1[i] = m.second.from;
      }

      for (size_t i = m.second.from; i <= m.second.to; ++i) {
        arr2[i] = m.first.from;
      }
    }

    void WriteAlignedTextToFile(const std::string &output_path, 
                                const utils::matches_vec &matches,
                                const std::vector<std::string> &text1_doc,
                                const std::vector<std::string> &text2_doc) {

      std::stringstream ss;
      utils::CompressedWriter gw(output_path);
      for (auto m: matches) {
        ss.str("");

        for (size_t i = m.first.from; i <= m.first.to; ++i) {
          ss << text1_doc.at(i);
        }

        ss << "\t";

        for (size_t i = m.second.from; i <= m.second.to; ++i) {
          ss << text2_doc.at(i);
        }
 
 	ss << "\t";

        ss << m.score;

        ss << "\n";

        std::string line = ss.str();
        gw.write(line);
      }
    }


} // namespace align
