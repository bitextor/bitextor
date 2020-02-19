#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <thread>
#include <memory>
#include "boost/program_options.hpp"
#include "document.h"
#include "blocking_queue.h"

using namespace bitextor;
using namespace std;

namespace po = boost::program_options;


void print_score(float score, Document const &left, Document const &right)
{
	// TODO: Don't print concurrently
	cout << score
	     << '\t' << left.url
	     << '\t' << right.url
	     << '\n';
}

size_t read_df(ifstream &fin, unordered_map<NGram, size_t> &df) {
	size_t n = 0;
	
	while (true) {
		Document document;
		
		if (!(fin >> document))
			break;
		
		for (auto const &entry : document.vocab)
			df[entry.first] += 1;
		
		++n;
	}
	
	return n;
}

size_t read_document_refs(ifstream &fin_tokens, ifstream &fin_urls, unordered_map<NGram,size_t> df, size_t document_cnt, vector<DocumentRef>::iterator it) {
	size_t n = 0;
	
	while (true) {
		Document buffer;
		
		if (!(fin_tokens >> buffer))
			break;
		
		if (!(fin_urls >> buffer.url)) { // TODO: Dangerous assumption that there is no space in url
			cerr << "Error while reading the url for the " << n << "th document" << endl;
			break;
		}

		*it++ = calculate_tfidf(buffer, document_cnt, df);
		++n;
	}
	
	return n;
}

int main(int argc, char *argv[]) {
	unsigned int n_threads = thread::hardware_concurrency() - 1;
	
	float threshold = 0.7;
	
	po::positional_options_description arg_desc;
	arg_desc.add("translated-tokens", 1);
	arg_desc.add("translated-urls", 1);
	arg_desc.add("english-tokens", 1);
	arg_desc.add("english-urls", 1);
	
	po::options_description opt_desc("Additional options");
	opt_desc.add_options()
		("help", "produce help message")
		("threads", po::value<unsigned int>(&n_threads), "set number of threads")
		("threshold", po::value<float>(&threshold), "set score threshold")
		("translated-tokens", po::value<string>(), "set input filename")
		("translated-urls", po::value<string>(), "set input filename")
		("english-tokens", po::value<string>(), "set input filename")
		("english-urls", po::value<string>(), "set input filename");
	
	po::variables_map vm;
	
	try {
		po::store(po::command_line_parser(argc, argv).options(opt_desc).positional(arg_desc).run(), vm);
		po::notify(vm);
	} catch (const po::error &exception) {
		cerr << exception.what() << endl;
		return 1;
	}
	
	if (vm.count("help")
		|| !vm.count("translated-tokens") || !vm.count("translated-urls")
		|| !vm.count("english-tokens") || !vm.count("english-urls"))
	{
		cout << "Usage: " << argv[0]
		     << " TRANSLATED-TOKENS TRANSLATED-URLS ENGLISH-TOKENS ENGLISH-URLS\n\n"
		     << opt_desc << endl;
		return 1;
	}
	
	// Read first set of documents into memory.

	ifstream tokens_in(vm["translated-tokens"].as<std::string>());
	if (!tokens_in) {
		cerr << "Could not read " << vm["translated-tokens"].as<std::string>() << endl;
		return 1;
	}
	
	ifstream urls_in(vm["translated-urls"].as<std::string>());
	if (!urls_in) {
		cerr << "Could not read " << vm["translated-urls"].as<std::string>() << endl;
		return 1;
	}

	ifstream en_tokens_in(vm["english-tokens"].as<std::string>());
	if (!en_tokens_in) {
		cerr << "Could not read " << vm["english-tokens"].as<std::string>() << endl;
		return 1;
	}
	
	ifstream en_urls_in(vm["english-urls"].as<std::string>());
	if (!en_urls_in) {
		cerr << "Could not read " << vm["english-urls"].as<std::string>() << endl;
		return 1;
	}

	// Calculate the document frequency for terms.
	// We'll use in_document_cnt later to reserve some space for the documents
	// we want to keep in memory.
	unordered_map<NGram,size_t> df;
	size_t in_document_cnt = read_df(tokens_in, df);
	size_t en_document_cnt = read_df(en_tokens_in, df);
	size_t document_cnt = in_document_cnt + en_document_cnt;
	
	// Rewind the input
	
	tokens_in.clear();
	tokens_in.seekg(0);
	en_tokens_in.clear();
	en_tokens_in.seekg(0);
	
	cerr << "Calculated DF from " << document_cnt << " documents" << endl;
	
	// Calculate TF/DF over the documents we have in memory
	std::vector<DocumentRef> refs(in_document_cnt);
	read_document_refs(tokens_in, urls_in, df, document_cnt, refs.begin());
	
	cerr << "Read " << refs.size() << " documents into memory" << endl;
	
	// Start reading the other set of documents we match against
	// (Note: they are not included in the DF table!)
	
	atomic<size_t> hits(0);
	
	vector<thread> consumers;
	
	blocking_queue<unique_ptr<Document>> queue(n_threads * 4);
	
	for (unsigned int n = 0; n < n_threads; ++n)
		consumers.push_back(thread([&queue, &refs, &df, &hits, threshold]() {
			while (true) {
				unique_ptr<Document> buffer(queue.pop());
				
				// Empty doc is poison
				if (!buffer)
					break;
				
				DocumentRef const &ref = calculate_tfidf(*buffer, refs.size(), df);
				
				for (auto const &document_ref : refs) {
					float score = calculate_alignment(document_ref, ref);
					
					if (score >= threshold)
						// print_score(score, document, buffer);
						++hits;
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
	
	for (size_t n = 0; true; ++n) {
		unique_ptr<Document> buffer(new Document());
		
		// If reading failed, we're probably at end of file
		if (!(en_tokens_in >> *buffer))
			break;
		
		if (!(en_urls_in >> buffer->url)) {
			cerr << "Error while reading url for the " << n << "th document" << endl;
			stop();
			return 3;
		}
		
		if (buffer->vocab.empty()) {
			cerr << "Reading the " << n << "th document resulted in an empty vocab" << endl;
			stop();
			return 4;
		}
		
		// Push this document to the alignment score calculators
		queue.push(std::move(buffer));
	}
	
	stop();
	
	// Tada!
	cout << hits.load() << endl;
}
