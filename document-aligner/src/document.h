#pragma once
#include "util/string_piece.hh"
#include "ngram.h"
#include <istream>
#include <unordered_map>
#include <vector>

namespace bitextor {

struct WordScore {
	NGram hash;
	float tfidf;
};

struct Document {
	// Document offset, used as identifier
	size_t id;
	
	// ngram frequency in document
	std::unordered_map<NGram, size_t> vocab;
};

struct DocumentRef {
	// Document offset, used as identifier
	size_t id;
	
	// ngram scores as a sorted array for quick sparse dot product
	std::vector<WordScore> wordvec;
};

// Assumes base64 encoded still.
void ReadDocument(const util::StringPiece &encoded, Document &to, size_t ngram_size);

void calculate_tfidf(Document const &document, DocumentRef &document_ref, size_t document_count, std::unordered_map<NGram, size_t> const &df);

} // namespace bitextor
