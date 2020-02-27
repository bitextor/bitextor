#pragma once
#include "util/string_piece.hh"
#include <istream>
#include <unordered_map>
#include <vector>

namespace bitextor {

struct WordScore {
	uint64_t hash; // Same as NGram::hash
	float tfidf;
};

struct Document {
	// Document offset, used as identifier
	size_t id;
	
	// ngram frequency in document
	std::unordered_map<uint64_t, size_t> vocab;
};

struct DocumentRef {
	// Document offset, used as identifier
	size_t id;
	
	// ngram scores as a sorted array for quick sparse dot product
	std::vector<WordScore> wordvec;
};

// Assumes base64 encoded still.
void ReadDocument(const StringPiece &encoded, Document &to, size_t ngram_size);

std::ostream &operator<<(std::ostream &stream, Document const &document);

std::ostream &operator<<(std::ostream &stream, DocumentRef const &ref);

void calculate_tfidf(Document const &document, DocumentRef &document_ref, size_t document_count, std::unordered_map<uint64_t, size_t> const &df);

float calculate_alignment(DocumentRef const &left, DocumentRef const &right);

} // namespace bitextor
