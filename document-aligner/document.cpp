#include "document.h"
#include "base64.h"
#include "ngram.h"
#include <sstream>
#include <iostream>
#include <cmath>


using namespace std;

namespace bitextor {

/**
 * Reads a single line of base64 encoded document into a Document.
 * TODO: Uses hard coded ngram size of 3 at the moment.
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

/**
 * Ostream helper for printing documents, for debugging
 */
ostream &operator<<(ostream &stream, Document const &document)
{
	stream << "--- Document ---\n";

	for (auto const &entry : document.vocab)
		stream << entry.first << ": " << entry.second << "\n";

	return stream << "--- end ---";
}
	
/**
 * Calculate TF/DF based on how often an ngram occurs in this document and how often it occurs at least once
 * across all documents. Only terms that are seen in this document and in the document frequency table are
 * counted. All other terms are ignored.
 */
void calculate_tfidf(Document &document, size_t document_count, map<NGram,size_t> const &df) {
	auto word_it = document.vocab.cbegin(),
		 word_end = document.vocab.cend();
	
	auto df_it = df.cbegin(),
		 df_end = df.cend();
	
	while (word_it != word_end && df_it != df_end) {
		if (word_it->first < df_it->first)
			++word_it;
		else if (df_it->first < word_it->first)
			++df_it;
		else {
			// Note: Matches tf_smooth setting 14 (2 for TF and 2 for IDF) of the python implementation
			document.wordvec.push_back(WordScore{
				.hash = word_it->first.hash,
				.tfidf = (float) log(word_it->second + 1) * log(document_count / (1.0f + df_it->second))
			});
			++word_it;
			++df_it;
		}
	}
}

/**
 * Dot product of two documents (of their ngram frequency really)
 */
float calculate_alignment(Document const &left, Document const &right) {
	float score = 0;
	
	auto lit = left.wordvec.cbegin(),
		 rit = right.wordvec.cbegin(),
		 lend = left.wordvec.cend(),
		 rend = right.wordvec.cend();
	
	while (lit != lend && rit != rend) {
		if (lit->hash < rit->hash)
			++lit;
		else if (rit->hash < lit->hash)
			++rit;
		else {
			score += lit->tfidf * rit->tfidf;
			++lit;
			++rit;
		}
	}
	
	return score;
}

} // namespace bitextor
