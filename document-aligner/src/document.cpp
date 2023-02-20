#include "document.h"
#include "base64.h"
#include "ngram.h"
#include <cmath>

using namespace std;

namespace bitextor {

/**
 * Reads a single line of base64 encoded document into a Document.
 */
void ReadDocument(const util::StringPiece &encoded, Document &document, size_t ngram_size)
{
	std::string body;
	base64_decode(encoded, body);

	document.vocab.clear();
	for (NGramIter ngram_it(body, ngram_size); ngram_it; ++ngram_it)
		document.vocab[*ngram_it] += 1;
}
	
inline float tfidf(size_t tf, size_t dc, size_t df) {
	// Note: Matches tf_smooth setting 14 (2 for TF and 2 for IDF) of the python implementation
	return logf(tf + 1) * logf(dc / (1.0f + df));
}
	
/**
 * Calculate TF/DF based on how often an ngram occurs in this document and how often it occurs at least once
 * across all documents. Only terms that are seen in this document and in the document frequency table are
 * counted. All other terms are ignored.
*/
void calculate_tfidf(Document const &document, DocumentRef &document_ref, size_t document_count, unordered_map<NGram, size_t> const &df, unordered_set<NGram> const &max_ngram_pruned) {
	document_ref.id = document.id;

	document_ref.wordvec.clear();
	document_ref.wordvec.reserve(document.vocab.size());
	
	float total_tfidf_l2 = 0;

	for (auto const &entry : document.vocab) {
		// How often does the term occur in the whole dataset?
		auto it = df.find(entry.first);

		float document_tfidf;

		if (it == df.end()) {
			if (max_ngram_pruned.find(entry.first) == max_ngram_pruned.end()) {
				document_tfidf = tfidf(entry.second, document_count, 1);
			}
			else{
				continue;
			}
		}
		else {
			document_tfidf = tfidf(entry.second, document_count, it->second);

			document_ref.wordvec.push_back(WordScore{
					.hash = entry.first,
					.tfidf = document_tfidf
			});
		}
		
		// Keep track of the squared sum of all values for L2 normalisation
		total_tfidf_l2 += document_tfidf * document_tfidf;
	}
	
	// Normalize
	total_tfidf_l2 = sqrt(total_tfidf_l2);
	for (auto &entry : document_ref.wordvec)
		entry.tfidf /= total_tfidf_l2;
}

} // namespace bitextor
