
#ifndef BLEUALIGN_CPP_MAIN_H
#define BLEUALIGN_CPP_MAIN_H


#include "src/utils/common.h"

#include <string>


void LoadExtracted(utils::umap_extracted &umap, const std::string &file_path);

bool LoadMatches(utils::matches_list &matches, const std::string &file_path, float threshold);

void LoadData(utils::AlignData &align_data, const utils::Config &cfg);

void Process(const utils::Config &cfg);

std::string MakeOutputPath(const std::string &path_dir, const std::string &suffix);

void WriteAlignedTextToFile(const std::string &output_dir, const utils::matches_list &matches);


#endif //BLEUALIGN_CPP_MAIN_H
