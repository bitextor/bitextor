#pragma once
#include "ngram.h"
#include <istream>
#include <map>

namespace bitextor {

struct WordScore {
	uint64_t hash; // Same as NGram::hash
	float tfidf;
};

struct Document {
	// Document URL, used as identifier
	std::string url;
	
	// Document body for book keeping/debugging
	// TODO: Maybe we can do without, same a lot of memory
	std::string body;
	
	// ngram frequency in document
	std::map<NGram, size_t> vocab;
	
	// ngram scores as a sorted array for quick sparse dot product
	std::vector<WordScore> wordvec;
};

std::istream &operator>>(std::istream &stream, Document &document);

std::ostream &operator<<(std::ostream &stream, Document const &document);

void calculate_tfidf(Document &document, size_t document_count, std::map<NGram,size_t> const &df);

float calculate_alignment(Document const &left, Document const &right);

} // namespace bitextor
