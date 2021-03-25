#include "ngram.h"
#include "murmur_hash.h"
#include <sstream>
#include <iostream>

using namespace std;

namespace bitextor {

NGramIter::NGramIter() {
	//
}

NGramIter::NGramIter(const util::StringPiece &source, size_t ngram_size)
: token_it_(source, " \n"), // Break on newline as well; I don't care about line beginnings and endings right now
  ngram_size_(ngram_size),
  pos_(0),
  end_(false),
  buffer_(ngram_size) {
	init();
}

void NGramIter::init() {
	for (pos_ = 0; pos_ < ngram_size_ - 1 && token_it_; ++pos_, ++token_it_)
		buffer_[pos_] = MurmurHashNative(token_it_->data(), token_it_->size(), 0);
	
	// Some documents are just too short
	if (!token_it_)
		end_ = true;

	increment();
}

void NGramIter::increment() {
	if (!token_it_) {
		end_ = true;
		return;
	}
	
	// Read next word & store hash
	buffer_[pos_ % ngram_size_] = MurmurHashNative(token_it_->data(), token_it_->size(), 0);
	
	// Create hash from combining past N word hashes
	ngram_.hash = 0;
	for (long offset = ngram_size_ - 1; offset >= 0; --offset)
		ngram_.hash = MurmurHashCombine(buffer_[(pos_ - offset) % ngram_size_], ngram_.hash);
	
	++pos_;
	++token_it_;
}

} // namespace bitextor
