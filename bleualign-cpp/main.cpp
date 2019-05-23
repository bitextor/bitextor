
#include "main.h"
#include "src/align.h"
#include "src/utils/common.h"
#include "src/utils/CompressedWriter.h"
#include "src/utils/logging.h"
#include "src/utils/string_to_float.h"

#include "util/string_piece.hh"
#include "util/file_piece.hh"
#include "util/read_compressed.hh"
#include "util/exception.hh"

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


namespace po = boost::program_options;

std::string MungeFilePath(const std::string &filePath)
{
  if (boost::filesystem::exists(filePath)) {
    return filePath;
  }
  else if (boost::filesystem::exists(filePath + ".gz")) {
    return filePath + ".gz";
  }
#ifdef XZ_COMPRESS
  else if (boost::filesystem::exists(filePath + ".xz")) {
    return filePath + ".xz";
  }
#endif
  else {
    UTIL_THROW(util::FileOpenException, "File does not exist");
  }

}

void LoadExtracted(utils::umap_extracted &umap, const std::string &file_path) {
  util::FilePiece in(MungeFilePath(file_path).c_str());
  std::vector<StringPiece> split_line;

  StringPiece l;
  while (in.ReadLineOrEOF(l)) {
    split_line.clear();
    utils::SplitStringPiece(split_line, l, '\t', 0, 2);

    std::string key = utils::PieceToString(split_line.at(0));
    umap[key].push_back(utils::PieceToString(split_line.at(1)));
  }

}


bool LoadMatches(utils::matches_list &matches, const std::string &file_path, float threshold) {
  try {
    util::FilePiece in(MungeFilePath(file_path).c_str());

    std::vector<StringPiece> split_line;

    StringPiece l;
    while (in.ReadLineOrEOF(l)) {
      split_line.clear();
      utils::SplitStringPiece(split_line, l, '\t', 0, 3);

      if (utils::ToFloat(split_line.at(0)) >= threshold)
        matches.push_back(std::make_pair(utils::PieceToString(split_line.at(1)), utils::PieceToString(split_line.at(2))));
    }

    return true;
  }
  catch (...) {
    return false;
  }
}


void LoadData(utils::AlignData &align_data, const utils::Config &cfg) {
  bool matchLoaded = LoadMatches(align_data.matches, cfg.matches_path, cfg.doc_threshold);

  if (matchLoaded) {
    LoadExtracted(align_data.umap_text1, cfg.text1_path);
    LoadExtracted(align_data.umap_text2, cfg.text2_path);
    LoadExtracted(align_data.umap_text1translated, cfg.text1_translated_path);
  }
}


void Process(const utils::Config &cfg) {
  utils::AlignData align_data;
  LoadData(align_data, cfg);

  for (size_t i = 0; i < align_data.matches.size(); ++i) {
    std::string output_path = MakeOutputPath(cfg.output_dir, std::to_string(i));
    align::AlignDocuments(output_path, align_data, align_data.matches.at(i).first, align_data.matches.at(i).second, cfg.bleu_threshold);
  }

  WriteAlignedTextToFile(cfg.output_dir, align_data.matches);

}


std::string MakeOutputPath(const std::string &path_dir, const std::string &suffix) {
  std::stringstream ss;
  ss << path_dir << "/aligned." << suffix << ".gz";
  return ss.str();
}

void WriteAlignedTextToFile(const std::string &output_dir, const utils::matches_list &matches) {

  std::stringstream ss;
  utils::CompressedWriter gw(output_dir + "/align.info.gz");
  for (size_t i = 0; i < matches.size(); ++i) {
    ss.str("");

    ss << std::to_string(i) << "\t";
    ss << matches.at(i).first << "\t";
    ss << matches.at(i).second << "\t";
    ss << "\n";

    std::string line = ss.str();
    gw.write(line);
  }
}


int main(int argc, char *argv[]) {

  utils::init();

  po::options_description desc("Allowed options");
  desc.add_options()
          ("help", "produce help message")
          ("text1", po::value<std::string>()->required(), "path to the first text file")
          ("text2", po::value<std::string>()->required(), "path to the second text file")
          ("text1translated", po::value<std::string>()->required(), "path to the translated text file (text1 to text2)")
          ("output-dir", po::value<std::string>()->required(), "path to the output directory")
          ("matches", po::value<std::string>()->required(),
           "path to a file containing matched documents. Format: score <tab> uri(text1) <tab> uri(text2)")
          ("doc-threshold", po::value<float>()->default_value(0.0f),
           "threshold for the matched documents (documents with a lower threshold are skipped)")
          ("bleu-threshold", po::value<float>()->default_value(0.0f), "BLEU threshold for matched sentences");


  po::variables_map vm;
  po::store(po::parse_command_line(argc, reinterpret_cast<const char *const *>(argv), desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << desc << std::endl;
    return 1;
  }

  utils::Config cfg;
  cfg.text1_path = vm["text1"].as<std::string>();
  cfg.text2_path = vm["text2"].as<std::string>();
  cfg.text1_translated_path = vm["text1translated"].as<std::string>();
  cfg.output_dir = vm["output-dir"].as<std::string>();
  cfg.matches_path = vm["matches"].as<std::string>();
  cfg.doc_threshold = vm["doc-threshold"].as<float>();
  cfg.bleu_threshold = vm["bleu-threshold"].as<float>();

  if (!boost::filesystem::exists(cfg.output_dir)) {
    std::cerr << "Output directory does not exist!" << std::endl;
    return 1;
  }

  if (!boost::filesystem::is_directory(cfg.output_dir)) {
    std::cerr << "Output path is not a directory!" << std::endl;
    return 1;
  }

  Process(cfg);

  return 0;
}
