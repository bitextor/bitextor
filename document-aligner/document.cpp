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

	string body = base64_decode(line);

	istringstream token_stream(body);
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
	stream << "--- Document ---\n" << document.url << "\n";

	for (auto const &entry : document.vocab)
		stream << entry.first << ": " << entry.second << "\n";

	return stream << "--- end ---";
}
	
ostream &operator<<(ostream &stream, DocumentRef const &document)
{
	stream << "--- Document Ref ---\n" << document.url << "\n";

	for (auto const &entry : document.wordvec)
		stream << entry.hash << ": " << entry.tfidf << "\n";

	return stream << "--- end ---";
}
	
inline float tfidf(size_t tf, size_t dc, size_t df) {
	// Note: Matches tf_smooth setting 14 (2 for TF and 2 for IDF) of the python implementation
	return (float) log(tf + 1) * log(dc / (1.0f + df));
}
	
/**
 * Calculate TF/DF based on how often an ngram occurs in this document and how often it occurs at least once
 * across all documents. Only terms that are seen in this document and in the document frequency table are
 * counted. All other terms are ignored.
*/
DocumentRef calculate_tfidf(Document &document, size_t document_count, unordered_map<NGram,size_t> const &df) {
	DocumentRef document_ref{
		.url = document.url
	};
	
	// With the following method we know that each word will get a score so
	// lets just reserve that space right now!
	document_ref.wordvec.reserve(document.vocab.size());
	
	for (auto const &entry : document.vocab) {
		auto it = df.find(entry.first);
	
		size_t term_df = it == df.end() ? 1 : it->second;
		
		document_ref.wordvec.push_back(WordScore{
			.hash = entry.first.hash,
			.tfidf = tfidf(entry.second, document_count, term_df)
		});
	}
	
	return document_ref;
}

/**
 * Dot product of two documents (of their ngram frequency really)
 */
float calculate_alignment(DocumentRef const &left, DocumentRef const &right) {
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
