
#ifndef FAST_BLEUALIGN_ALIGN_H
#define FAST_BLEUALIGN_ALIGN_H


#include "search.h"
#include "util/string_piece.hh"
#include "utils/common.h"

#include <string>
#include <memory>
#include <vector>

namespace align {

    void AlignDocuments(const std::string &output_path, const utils::AlignData &align_data, const StringPiece matched_text1_uri,
                        StringPiece matched_text2_uri, double threshold);

    void Align(utils::matches_vec &matches, const std::vector<std::string> &text1translated_doc,
               const std::vector<std::string> &text2_doc, double threshold);

    void EvalSents(std::vector<utils::scoremap> &scorelist, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, unsigned short ngram_size, size_t maxalternatives);

    void GapFiller(utils::matches_vec &matched, const std::vector<std::string> &text1translated_doc,
                   const std::vector<std::string> &text2_doc, size_t gap_limit, double threshold);

    void ProduceMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, size_t from, size_t to, size_t limit,
                                bool reverse = false);

    void PreGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                               const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr, size_t pos,
                               size_t gap_limit);

    void PostGapMergedSentences(std::vector<std::string> &merged_text, utils::vec_pair &merged_pos,
                                const std::vector<std::string> &docs, std::unique_ptr<int[]> &matches_arr, size_t pos,
                                size_t matches_arr_size, size_t gap_limit);

    void FillMatches(std::unique_ptr<int[]> &arr1, std::unique_ptr<int[]> &arr2, utils::match m);

    void WriteAlignedTextToFile(const std::string &output_path, const utils::matches_vec &matches,
                                const std::vector<std::string> &text1_doc, const std::vector<std::string> &text2_doc);


} // namespace align

#endif //FAST_BLEUALIGN_ALIGN_H
