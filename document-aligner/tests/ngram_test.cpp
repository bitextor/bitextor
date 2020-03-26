#define BOOST_TEST_MODULE example
#include <vector>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../src/ngram.h"
#include "../src/murmur_hash.h"

using namespace std;
using namespace bitextor;

NGram make_ngram(vector<string> const &words)
{
	uint64_t hash = 0;

	for (string const &word : words)
		hash = MurmurHashCombine(MurmurHashNative(word.data(), word.size(), 0), hash);

	return hash;
}

BOOST_AUTO_TEST_CASE(test_ngram_it_1)
{
	string document = "Hello this is a test";
	
	vector<NGram> ngrams;
	for (NGramIter iter(StringPiece(document.data(), document.size()), 3); iter; ++iter)
		ngrams.push_back(*iter);

	vector<NGram> expected{
		make_ngram({"Hello", "this", "is"}),
		make_ngram({"this", "is", "a"}),
		make_ngram({"is", "a", "test"})
	};

	BOOST_TEST(ngrams == expected, boost::test_tools::per_element());
}