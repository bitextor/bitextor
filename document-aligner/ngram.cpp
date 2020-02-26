#include "ngram.h"
#include "murmur_hash.h"
#include <sstream>
#include <iostream>

using namespace std;

namespace bitextor {

NGramIter::NGramIter() {
	//
}

NGramIter::NGramIter(StringPiece const &source, size_t ngram_size)
: token_it_(source, " \n"),
  ngram_size_(ngram_size),
  pos_(0),
  end_(false),
  buffer_(ngram_size) {
	init();
}

void NGramIter::init() {
	for (pos_ = 0; pos_ < ngram_size_ - 1; ++pos_, ++token_it_)
		buffer_[pos_] = MurmurHashNative(token_it_->data(), token_it_->size(), 0);
}

void NGramIter::increment() {
	if (!token_it_) {
		end_ = true;
		return;
	}
	
	// Read next word & store hash
	buffer_[pos_ % ngram_size_] = MurmurHashNative(token_it_->data(), token_it_->size(), 0);
	
	// Create hash from combining past N word hashes
	ngram_hash_ = 0;
	for (long offset = ngram_size_ - 1; offset >= 0; --offset)
		ngram_hash_ = MurmurHashCombine(buffer_[(pos_ - offset) % ngram_size_], ngram_hash_);
	
	++pos_;
	++token_it_;
}

} // namespace bitextor
