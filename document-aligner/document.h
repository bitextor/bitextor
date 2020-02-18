#pragma once
#include "ngram.h"
#include <istream>
#include <map>

namespace bitextor {

struct Statistics {
	size_t count;
	float tfidf;
};

struct Document {
	std::string url;
	std::string body;
	std::map<NGram, Statistics> vocab;
};

struct WordScore {
	uint64_t hash; // Same as NGram::hash
	float tfidf;
};

std::istream &operator>>(std::istream &stream, Document &document);

std::ostream &operator<<(std::ostream &stream, Document const &document);

float DocumentDot(const WordScore *first, const WordScore *first_end, const WordScore *second, const WordScore *second_end);

} // namespace bitextor
