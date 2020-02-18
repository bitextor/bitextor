#include "document.h"
#include <iostream>
#include <fstream>
#include <map>
#include "boost/program_options.hpp"

using namespace bitextor;
using namespace std;

namespace po = boost::program_options;

void calculate_tfidf(Document &document, size_t document_count, map<NGram,size_t> const &df) {
	auto word_it = document.vocab.begin(),
	     word_end = document.vocab.end();
	
	auto df_it = df.cbegin(),
	     df_end = df.cend();
	
	while (word_it != word_end && df_it != df_end) {
		if (word_it->first < df_it->first)
			++word_it;
		else if (df_it->first < word_it->first)
			++df_it;
		else {
			word_it->second.tfidf = (float) word_it->second.count * (document_count / (1.0f * df_it->second));
			++word_it;
			++df_it;
		}
	}
}

float calculate_alignment(Document const &left, Document const &right) {
	float score = 0;
	
	auto lit = left.vocab.cbegin(),
	     rit = right.vocab.cbegin(),
	     lend = left.vocab.cend(),
	     rend = right.vocab.cend();
	
	while (lit != lend && rit != rend) {
		if (lit->first < rit->first)
			++lit;
		else if (rit->first < lit->first)
			++rit;
		else {
			score += lit->second.tfidf * rit->second.tfidf;
			++lit;
			++rit;
		}
	}
	
	return score;
}

int main(int argc, char *argv[]) {
	po::positional_options_description arg_desc;
	arg_desc.add("translated-tokens", 1);
	arg_desc.add("translated-urls", 1);
	arg_desc.add("english-tokens", 1);
	arg_desc.add("english-urls", 1);
	
	po::options_description opt_desc("Additional options");
	opt_desc.add_options()
		("help", "produce help message")
		("threshold", po::value<float>()->default_value(0.7), "set score threshold")
		("translated-tokens", po::value<string>(), "set input filename")
		("translated-urls", po::value<string>(), "set input filename")
		("english-tokens", po::value<string>(), "set input filename")
		("english-urls", po::value<string>(), "set input filename");
	
	po::variables_map vm;
	
	try {
		po::store(po::command_line_parser(argc, argv).options(opt_desc).positional(arg_desc).run(), vm);
		po::notify(vm);
	} catch (const po::error &exception) {
		cerr << exception.what() << endl;
		return 1;
	}
		
	if (vm.count("help")
		|| !vm.count("translated-tokens") || !vm.count("translated-urls")
		|| !vm.count("english-tokens") || !vm.count("english-urls"))
	{
		cout << "Usage: " << argv[0]
		     << " TRANSLATED-TOKENS TRANSLATED-URLS ENGLISH-TOKENS ENGLISH-URLS\n\n"
		     << opt_desc << endl;
		return 1;
	}

	std::vector<Document> documents;
	
	ifstream tokens_in(vm["translated-tokens"].as<std::string>());
	ifstream urls_in(vm["translated-urls"].as<std::string>());

	Document buffer;

	while (tokens_in >> buffer) {
		if (!(urls_in >> buffer.url)) { // TODO: Dangerous assumption that there is no space in url
			cerr << "Error while reading the url for the " << documents.size() << "th document" << endl;
			return 2;
		}

		documents.push_back(buffer);
	}

	cerr << "Read " << documents.size() << " documents" << endl;
	
	map<NGram,size_t> df;
	
	for (auto const &document : documents)
		for (auto const &entry : document.vocab)
			df[entry.first] += 1;
	
	cerr << "Aggregated DF" << endl;
	
	for (auto &document : documents)
		calculate_tfidf(document, documents.size(), df);
	
	cerr << "Calculated translated TFIDF scores" << endl;
	
	ifstream en_tokens_in(vm["english-tokens"].as<std::string>());
	ifstream en_urls_in(vm["english-urls"].as<std::string>());
	
	float threshold = vm["threshold"].as<float>();
	
	size_t n = 0;

	while (en_tokens_in >> buffer) {
		if (!(en_urls_in >> buffer.url)) {
			cerr << "Error while reading url for the " << n << "th document" << endl;
			return 3;
		}
		
		++n;
		
		calculate_tfidf(buffer, documents.size(), df);
		
		for (auto const &document : documents) {
			float score = calculate_alignment(document, buffer);
			
			if (score < threshold)
				continue;
			
			cout << score
				 << '\t' << document.url
				 << '\t' << buffer.url
				 << '\n';
		}
	}
}
