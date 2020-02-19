#pragma once
#include "ngram.h"
#include <istream>
#include <map>
#include <unordered_map>

namespace bitextor {

struct WordScore {
	uint64_t hash; // Same as NGram::hash
	float tfidf;
};

struct Document {
	// Document URL, used as identifier
	std::string url;
	
	// ngram frequency in document
	std::map<NGram, size_t> vocab;
};

struct DocumentRef {
	// Document URL, used as identifier
	std::string url;
	
	// ngram scores as a sorted array for quick sparse dot product
	std::vector<WordScore> wordvec;
};

std::istream &operator>>(std::istream &stream, Document &document);

std::ostream &operator<<(std::ostream &stream, Document const &document);

std::ostream &operator<<(std::ostream &stream, DocumentRef const &ref);

DocumentRef calculate_tfidf(Document &document, size_t document_count, std::unordered_map<NGram,size_t> const &df);

float calculate_alignment(DocumentRef const &left, DocumentRef const &right);

} // namespace bitextor
