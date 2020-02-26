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
 */
void ReadDocument(const StringPiece &encoded, Document &document, size_t ngram_size)
{
	std::string body;
	base64_decode(encoded, body);
	for (NGramIter ngram_it(body, ngram_size); ngram_it; ++ngram_it)
		document.vocab[*ngram_it] += 1;
}

/**
 * Ostream helper for printing documents, for debugging
 */
ostream &operator<<(ostream &stream, Document const &document)
{
	stream << "--- Document ---\n" << document.id << "\n";

	for (auto const &entry : document.vocab)
		stream << entry.first << ": " << entry.second << "\n";

	return stream << "--- end ---";
}
	
ostream &operator<<(ostream &stream, DocumentRef const &document)
{
	stream << "--- Document Ref ---\n" << document.id << "\n";

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
DocumentRef calculate_tfidf(Document &document, size_t document_count, unordered_map<uint64_t, size_t> const &df) {
	DocumentRef document_ref{
		.id = document.id
	};
	
	// With the following method we know that each word will get a score so
	// lets just reserve that space right now!
	document_ref.wordvec.reserve(document.vocab.size());
	
	float total_tfidf_l2 = 0;
	
	for (auto const &entry : document.vocab) {
		// How often does the term occur in the whole dataset?
		auto it = df.find(entry.first);
	
		// If we can't find it (e.g. because we didn't really read the whole
		// dataset) we just assume one: just this document.
		size_t term_df = it == df.end() ? 1 : it->second;
	
		float document_tfidf = tfidf(entry.second, document_count, term_df);
		
		// Keep track of the squared sum of all values for L2 normalisation
		total_tfidf_l2 += document_tfidf * document_tfidf;
		
		document_ref.wordvec.push_back(WordScore{
			.hash = entry.first,
			.tfidf = document_tfidf
		});
	}
	
	// Sort wordvec, which is assumed by calculate_alignment
	sort(document_ref.wordvec.begin(),
		 document_ref.wordvec.end(),
		 [] (WordScore const &lft, WordScore const &rgt) {
		return lft.hash < rgt.hash;
	});
	
	// Normalize
	
	total_tfidf_l2 = sqrt(total_tfidf_l2);
	for (auto &entry : document_ref.wordvec)
		entry.tfidf /= total_tfidf_l2;
	
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
