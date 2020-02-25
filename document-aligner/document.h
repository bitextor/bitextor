#pragma once
#include "ngram.h"
#include "util/string_piece.hh"
#include <istream>
#include <map>
#include <unordered_map>

namespace bitextor {

struct WordScore {
	uint64_t hash; // Same as NGram::hash
	float tfidf;
};

struct Document {
	// Document offset, used as identifier
	size_t id;
	
	// ngram frequency in document
	std::map<NGram, size_t> vocab;
};

struct DocumentRef {
	// Document offset, used as identifier
	size_t id;
	
	// ngram scores as a sorted array for quick sparse dot product
	std::vector<WordScore> wordvec;
	
	inline DocumentRef() {};
	
	inline DocumentRef(Document const &document) : id(document.id) {}
};

// Assumes base64 encoded still.
void ReadDocument(const StringPiece &encoded, Document &to);

std::ostream &operator<<(std::ostream &stream, Document const &document);

std::ostream &operator<<(std::ostream &stream, DocumentRef const &ref);

DocumentRef calculate_tfidf(Document &document, size_t document_count, std::unordered_map<NGram,size_t> const &df);

float calculate_alignment(DocumentRef const &left, DocumentRef const &right);

} // namespace bitextor
