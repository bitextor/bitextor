#include "ngram.h"
#include "murmur_hash.h"
#include <vector>

using namespace std;

namespace bitextor {

ingramstream::ingramstream(istream &stream, size_t size)
:
	istream(stream.rdbuf()),
	_size(size),
	_offset(0),
	_buffer()
{
	//
}

NGram ingramstream::read_ngram()
{
	// First call: our buffer is still empty. Fill it up right till the
	// last word. Reading the last word is standard behaviour so outside
	// this condition.
	if (_buffer.empty()) {
		_buffer.resize(_size);

		for (size_t i = 0; i < _size - 1; ++i)
			*this >> _buffer[i];
	}

	// Read last token of ngram, put it at end of buffer
	*this >> _buffer[(_offset + _size - 1) % _size];

	// Start reading at start of buffer
	uint64_t hash(MurmurHashNative(_buffer[_offset].data(), _buffer[_offset].size(), 0));

	// Continue with the rest of the buffer (using start as seed)
	for (size_t i = 1; i < _size; ++i) {
		string const &token = _buffer[(_offset + i) % _size];
		hash = MurmurHashCombine(MurmurHashNative(token.data(), token.size(), 0), hash);
	}

	// Increase buffer offset (this is a -> is a test)
	_offset = (_offset + 1) % _size;
	
	return NGram{hash};
}

ingramstream &operator>>(ingramstream &stream, NGram &word)
{
	word = stream.read_ngram();
	return stream;
}

ostream &operator<<(ostream &str, vector<string> const &words)
{
	str << '[';

	auto it = words.cbegin();

	if (it != words.cend()) {
		str << *it;
		++it;
	}

	while (it != words.cend()) {
		str << ',' << *it;
		++it;
	}

	return str << ']';
}

ostream &operator<<(ostream &str, NGram const &gram) 
{
	return str << '{' << gram.hash << '}';
}

} // namespace bitextor
