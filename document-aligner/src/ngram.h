#pragma once
#include <vector>
#include <boost/iterator/iterator_facade.hpp>
#include <util/tokenize_piece.hh>

namespace bitextor {

struct NGram {
	uint64_t hash;

	inline bool operator==(NGram const &other) const {
		return hash == other.hash;
	}
};

class NGramIter : public boost::iterator_facade<NGramIter, const NGram, boost::forward_traversal_tag> {
public:
	NGramIter();
	NGramIter(const util::StringPiece &source, size_t ngram_size);
	
	inline bool operator!() const {
		return end_;
	}

	inline explicit operator bool() const {
		return !end_;
	}

private:
	friend class boost::iterator_core_access;

	util::TokenIter<util::AnyCharacter, true> token_it_;

	size_t ngram_size_;
	size_t pos_;
	bool end_;
	std::vector<uint64_t> buffer_;
	NGram ngram_;

	void init();
	void increment();

	inline bool equal(NGramIter const &other) const {
		return token_it_ == other.token_it_ && end_ == other.end_;
	}

	inline const NGram &dereference() const {
		UTIL_THROW_IF(end_, util::OutOfTokens, "We already reached end");
		return ngram_;
	}
};

} // namespace bitextor

namespace std {
	template <> struct hash<bitextor::NGram> {
		inline size_t operator()(bitextor::NGram const &val) const {
			return val.hash;
		}
	};
} // namespace std
