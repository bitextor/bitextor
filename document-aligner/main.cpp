#include "document.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace bitextor;
using namespace std;

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
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " tokens.txt urls.txt [ tokens2.txt urls2.txt ]" << endl;
		return 1;
	}

	std::vector<Document> documents;
	
	ifstream tokens_in(argv[1]);
	ifstream urls_in(argv[2]);

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
	
	if (argc >= 5) {
	
		ifstream en_tokens_in(argv[3]);
		ifstream en_urls_in(argv[4]);
		
		size_t n = 0;
	
		while (en_tokens_in >> buffer) {
			if (!(en_urls_in >> buffer.url)) {
				cerr << "Error while reading url for the " << n << "th document" << endl;
				return 3;
			}
			
			++n;
			
			calculate_tfidf(buffer, documents.size(), df);
			
			for (auto const &document : documents) {
				cout << calculate_alignment(document, buffer)
				     << '\t' << document.url
				     << '\t' << buffer.url
				     << '\n';
			}
		}
	}
}
