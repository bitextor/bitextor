#include <iostream>
#include <fstream>
#include <map>
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

int main(int argc, char *argv[]) {
	po::positional_options_description arg_desc;
	arg_desc.add("translated-tokens", 1);
	arg_desc.add("translated-urls", 1);
	arg_desc.add("english-tokens", 1);
	arg_desc.add("english-urls", 1);
	
	po::options_description opt_desc("Additional options");
	opt_desc.add_options()
		("help", "produce help message")
		("threshold", po::value<float>()->default_value(0.7), "set score threshold")
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

	std::vector<unique_ptr<Document>> documents;
	
	ifstream tokens_in(vm["translated-tokens"].as<std::string>());
	ifstream urls_in(vm["translated-urls"].as<std::string>());

	while (true) {
		unique_ptr<Document> buffer(new Document());
		
		if (!(tokens_in >> *buffer))
			break;
		
		if (!(urls_in >> buffer->url)) { // TODO: Dangerous assumption that there is no space in url
			cerr << "Error while reading the url for the " << documents.size() << "th document" << endl;
			return 2;
		}

		documents.push_back(std::move(buffer));
	}

	cerr << "Read " << documents.size() << " documents" << endl;
	
	// Calculate the document frequency for terms.
	
	map<NGram,size_t> df;
	
	for (auto const &document : documents)
		for (auto const &entry : document->vocab)
			df[entry.first] += 1;
	
	cerr << "Aggregated DF" << endl;
	
	// Calculate TF/DF over the documents we have in memory
	
	for (auto &document : documents) {
		// Turn the vocab map into a sorted tfidf score list
		calculate_tfidf(*document, documents.size(), df);
		
		// Save a bit of memory
		document->vocab.clear();
	}
	
	cerr << "Calculated translated TFIDF scores" << endl;
	
	// Start reading the other set of documents we match against
	// (Note: they are not included in the DF table!)
	
	ifstream en_tokens_in(vm["english-tokens"].as<std::string>());
	ifstream en_urls_in(vm["english-urls"].as<std::string>());
	
	float threshold = vm["threshold"].as<float>();
	
	size_t n = 0;
	
	atomic<size_t> hits(0);
	
	vector<thread> consumers;
	
	size_t n_threads = 4;
	
	blocking_queue<unique_ptr<Document>> queue(n_threads * 4);
	
	for (size_t n = 0; n < n_threads; ++n)
		consumers.push_back(thread([&queue, &documents, &hits, threshold]() {
			while (true) {
				unique_ptr<Document> buffer(queue.pop());
				
				// Empty doc is poison
				if (!buffer)
					break;
				
				for (auto const &document : documents) {
					float score = calculate_alignment(*document, *buffer);
					
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
	
	while (true) {
		unique_ptr<Document> buffer(new Document());
		
		if (!(en_tokens_in >> *buffer))
			break;
		
		if (!(en_urls_in >> buffer->url)) {
			cerr << "Error while reading url for the " << n << "th document" << endl;
			stop();
			return 3;
		}
		
		++n;
		
		if (buffer->vocab.empty()) {
			cerr << "Document " << n << " resulted in an empty vocab" << endl;
			stop();
			return 4;
		}
		
		// TODO: Move this into consumers as well?
		calculate_tfidf(*buffer, documents.size(), df);
		
		buffer->vocab.clear();
		
		// Make 100% sure it is not an empty document, empty docs are poisonous!
		if (buffer->wordvec.empty()) {
			cerr << "Document " << n << " resulted in an empty word vec" << endl;
			stop();
			return 5;
		}
		
		// Push this document to the alignment score calculators
		queue.push(std::move(buffer));
	}
	
	stop();
	
	// Tada!
	cout << hits.load() << endl;
}
