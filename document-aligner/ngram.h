#pragma once
#include <vector>
#include <istream>
#include <ostream>

namespace bitextor {

struct NGram {
	uint64_t hash; // TODO: uint32_t
	
	std::vector<std::string> tokens; // For debugging right now

	inline bool operator<(NGram const &other) const {
		return hash < other.hash;
	}

	inline bool operator==(NGram const &other) const {
		return hash == other.hash;
	}
};

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