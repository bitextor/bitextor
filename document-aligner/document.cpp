#include "document.h"
#include "base64.h"
#include "ngram.h"
#include <sstream>
#include <iostream>


using namespace std;

namespace bitextor {

/*
float DocumentDot(const WordScore *first, const WordScore *first_end, const WordScore *second, const WordScore *second_end)
{
	float ret = 0.0;
	while (first != first_end && second != second_end) {
		if (*first < *second) {
			++first;
		} else if (*first > *second) {
			++second;
		} else {
			ret += first->tfidf * second->tfidf;
			++first;
			++second;
		}
	}
	return ret;
}
*/

istream &operator>>(istream &stream, Document &document)
{
	string line;
	getline(stream, line);

	document.body = base64_decode(line);

	istringstream token_stream(document.body);
	ingramstream ngram_stream(token_stream, 3);

	NGram ngram;

	while (ngram_stream >> ngram) {
		document.vocab[ngram] += 1;
	}

	return stream;
}

ostream &operator<<(ostream &stream, Document const &document)
{
	stream << "--- Document ----\n";

	for (auto const &entry : document.vocab)
		stream << entry.first << ": " << entry.second << "\n";

	return stream << "--- end ---";
}

} // namespace bitextor