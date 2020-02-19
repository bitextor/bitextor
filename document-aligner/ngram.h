#pragma once
#include <vector>
#include <istream>
#include <ostream>

namespace bitextor {

/**
 * Struct around uint64_t to make it easy to add the original tokens as
 * a vector and to differentiate between types.
 */
struct NGram {
	uint64_t hash;
	
	inline bool operator<(NGram const &other) const {
		return hash < other.hash;
	}

	inline bool operator==(NGram const &other) const {
		return hash == other.hash;
	}
};

/**
 * Istream wrapper that reads space separated words into ngrams. Keeps
 * an internal buffer to do so. Use toghether with >> to read the ngrams
 * from this stream.
 */
class ingramstream : public std::istream {
public:
	ingramstream(std::istream &stream, size_t size);
	NGram read_ngram();
private:
	size_t _size;
	size_t _offset;
	std::vector<std::string> _buffer;
};

ingramstream &operator>>(ingramstream &stream, NGram &word);

std::ostream &operator<<(std::ostream &str, NGram const &gram);

} // namespace bitextor

namespace std {

template <> struct hash<bitextor::NGram> {
	inline std::size_t operator()(bitextor::NGram const &k) const {
		return k.hash;
	}
};

} // namespace std
