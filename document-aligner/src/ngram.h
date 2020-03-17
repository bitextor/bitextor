#pragma once
#include <vector>
#include <boost/iterator/iterator_facade.hpp>
#include <util/tokenize_piece.hh>

namespace bitextor {

class NGramIter : public boost::iterator_facade<NGramIter, const uint64_t, boost::forward_traversal_tag> {
public:
	NGramIter();
	NGramIter(StringPiece const &source, size_t ngram_size);
	
	inline bool operator!() const {
		return end_;
	}

	inline operator bool() const {
		return !end_;
	}

private:
	friend class boost::iterator_core_access;

	util::TokenIter<util::AnyCharacter, true> token_it_;

	size_t ngram_size_;
	size_t pos_;
	bool end_;
	std::vector<uint64_t> buffer_;
	uint64_t ngram_hash_;

	void init();
	void increment();

	inline bool equal(NGramIter const &other) const {
		return token_it_ == other.token_it_ && end_ == other.end_;
	}

	inline const uint64_t &dereference() const {
		UTIL_THROW_IF(end_, util::OutOfTokens, "We already reached end");
		return ngram_hash_;
	}
};

} // namespace bitextor
