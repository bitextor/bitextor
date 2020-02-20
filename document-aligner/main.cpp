#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <thread>
#include <memory>
#include <mutex>
#include <boost/program_options.hpp>
#include "document.h"
#include "transparent_fstream.h"
#include "blocking_queue.h"

using namespace bitextor;
using namespace std;

namespace po = boost::program_options;

mutex print_lock;

void print_score(float score, DocumentRef const &left, DocumentRef const &right)
{
	unique_lock<mutex> lock(print_lock);
	cout << fixed << setprecision(5)
	     << score
	     << '\t' << left.id
	     << '\t' << right.id
	     << '\n';
}

/**
 * Count the number of documents a term occurs in. Optinally it will only assess one in N documents (where
 * N is the skip_rate) but note that the document count it returns is for all documents, including the ones it
 * skipped. Because we are dividing by this number for TF/IDF it increments counts with skip_rate instead of
 * 1. So at the end of the day you can just do df / document_count to get the IDF.
 */
size_t read_df(istream &fin, unordered_map<NGram, size_t> &df, size_t skip_rate = 1) {
	
	// TODO: Can I make this thing multithreaded? Producer/consumer, but the
	// updating of the df map can't be concurrent, so the only profit would
	// be the base64-decode + ngram generation.
	
	// Number of documents actually read;
	size_t document_cnt = 0;
	
	while (true) {
		Document document;
		
		// If this is not a lucky line, skip over it and jump to next loop.
		if (document_cnt % skip_rate) {
			// If we couldn't skip forward, we're at end of file
			if (!fin.ignore(numeric_limits<std::streamsize>::max(), '\n'))
				break;
			
			++document_cnt;
			continue;
		}
		
		// If we can't read the next document, we must be at end of file.
		if (!(fin >> document))
			break;
		
		// Update global term counts based on which terms we saw in the doc
		for (auto const &entry : document.vocab)
			df[entry.first] += skip_rate;
		
		++document_cnt;
	}
	
	return document_cnt;
}

size_t read_document_refs(istream &fin_tokens, unordered_map<NGram,size_t> df, size_t document_cnt, vector<DocumentRef>::iterator it) {
	size_t n = 0;
	
	while (true) {
		Document buffer;
		
		if (!(fin_tokens >> buffer))
			break;
		
		buffer.id = ++n;

		*it++ = calculate_tfidf(buffer, document_cnt, df);
	}
	
	return n;
}

int score_documents(vector<DocumentRef> const &refs, unordered_map<NGram, size_t> const &df, istream &in_tokens, float threshold, unsigned int n_threads) {
	vector<thread> consumers;
	
	blocking_queue<unique_ptr<Document>> queue(n_threads * 4);
	
	for (unsigned int n = 0; n < n_threads; ++n)
		consumers.push_back(thread([&queue, &refs, &df, threshold]() {
			while (true) {
				unique_ptr<Document> buffer(queue.pop());
				
				// Empty doc is poison
				if (!buffer)
					break;
				
				DocumentRef const &ref = calculate_tfidf(*buffer, refs.size(), df);
				
				for (auto const &document_ref : refs) {
					float score = calculate_alignment(document_ref, ref);
					
					if (score >= threshold)
						print_score(score, document_ref, ref);
				}
			}
		}));
	
	auto stop = [&consumers, &queue, n_threads]() {
		// Send poison to all workers
		for (size_t n = 0; n < n_threads; ++n)
			queue.push(nullptr);
		
		// Wait for the workers to finish
		for (auto &consumer : consumers)
			consumer.join();
	};
	
	for (size_t n = 1; true; ++n) {
		unique_ptr<Document> buffer(new Document());
		
		// If reading failed, we're probably at end of file
		if (!(in_tokens >> *buffer))
			break;
		
		buffer->id = n;
		
		// Push this document to the alignment score calculators
		queue.push(std::move(buffer));
	}
	
	stop();
	
	return 0;
}

int main(int argc, char *argv[]) {
	unsigned int n_threads = thread::hardware_concurrency() - 1;
	
	float threshold = 0.1;
	
	size_t df_sample_rate = 1;
	
	po::positional_options_description arg_desc;
	arg_desc.add("translated-tokens", 1);
	arg_desc.add("english-tokens", 1);
	
	po::options_description opt_desc("Additional options");
	opt_desc.add_options()
		("help", "produce help message")
		("df-sample-rate", po::value<size_t>(&df_sample_rate), "set sample rate to every n-th document")
		("threads", po::value<unsigned int>(&n_threads), "set number of threads")
		("threshold", po::value<float>(&threshold), "set score threshold")
		("translated-tokens", po::value<string>(), "set input filename")
		("english-tokens", po::value<string>(), "set input filename");
	
	po::variables_map vm;
	
	try {
		po::store(po::command_line_parser(argc, argv).options(opt_desc).positional(arg_desc).run(), vm);
		po::notify(vm);
	} catch (const po::error &exception) {
		cerr << exception.what() << endl;
		return 1;
	}
	
	if (vm.count("help") || !vm.count("translated-tokens") || !vm.count("english-tokens")) {
		cout << "Usage: " << argv[0]
		     << " TRANSLATED-TOKENS ENGLISH-TOKENS\n\n"
		     << opt_desc << endl;
		return 1;
	}
	
	// Calculate the document frequency for terms.
	// We'll use in_document_cnt later to reserve some space for the documents
	// we want to keep in memory.
	unordered_map<NGram,size_t> df;
	
	size_t in_document_cnt, en_document_cnt;
	
	{
		transparent_istream in_tokens(vm["translated-tokens"].as<std::string>());
		in_document_cnt = read_df(in_tokens, df, df_sample_rate);
	}
	
	{
		transparent_istream en_tokens(vm["english-tokens"].as<std::string>());
		en_document_cnt = read_df(en_tokens, df, df_sample_rate);
	}
	
	size_t document_cnt = in_document_cnt + en_document_cnt;
	
	cerr << "Calculated DF from " << document_cnt / df_sample_rate << " documents" << endl;
	
	// Calculate TF/DF over the documents we have in memory
	std::vector<DocumentRef> refs(in_document_cnt);
	
	{
		transparent_istream in_tokens(vm["translated-tokens"].as<std::string>());
		read_document_refs(in_tokens, df, document_cnt, refs.begin());
	}
	
	cerr << "Read " << refs.size() << " documents into memory" << endl;
	
	// Start reading the other set of documents we match against
	// (Note: they are not included in the DF table!)
	
	transparent_istream en_tokens(vm["english-tokens"].as<std::string>());
	return score_documents(refs, df, en_tokens, threshold, n_threads);
}
