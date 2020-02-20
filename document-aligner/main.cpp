#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <thread>
#include <memory>
#include <mutex>
#include <boost/program_options.hpp>
#include "document.h"
#include "transparent_ifstream.h"
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
	     << '\t' << left.url
	     << '\t' << right.url
	     << '\n';
}

size_t read_df(istream &fin, unordered_map<NGram, size_t> &df) {
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

size_t read_df_from_file(string const &path, unordered_map<NGram, size_t> &df) {
	transparent_istream in_file(path);
	return read_df(in_file, df);
}

size_t read_document_refs(istream &fin_tokens, istream &fin_urls, unordered_map<NGram,size_t> df, size_t document_cnt, vector<DocumentRef>::iterator it) {
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

size_t read_document_refs_from_file(string const &path_tokens, string const &path_urls, unordered_map<NGram,size_t> df, size_t document_cnt, vector<DocumentRef>::iterator it) {
	transparent_istream in_tokens(path_tokens);
	transparent_istream in_urls(path_urls);
	return read_document_refs(in_tokens, in_urls, df, document_cnt, it);
}

int score_documents(vector<DocumentRef> const &refs, unordered_map<NGram, size_t> const &df, string const &path_tokens, string const &path_urls, float threshold, unsigned int n_threads) {
	vector<thread> consumers;
	
	blocking_queue<unique_ptr<Document>> queue(n_threads * 4);
	
	transparent_istream in_tokens(path_tokens);
	
	transparent_istream in_urls(path_urls);
	
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
	
	for (size_t n = 0; true; ++n) {
		unique_ptr<Document> buffer(new Document());
		
		// If reading failed, we're probably at end of file
		if (!(in_tokens >> *buffer))
			break;
		
		if (!(in_urls >> buffer->url)) {
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
	
	return 0;
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
	
	// Calculate the document frequency for terms.
	// We'll use in_document_cnt later to reserve some space for the documents
	// we want to keep in memory.
	unordered_map<NGram,size_t> df;
	size_t in_document_cnt = read_df_from_file(vm["translated-tokens"].as<std::string>(), df);
	size_t en_document_cnt = read_df_from_file(vm["english-tokens"].as<std::string>(), df);
	size_t document_cnt = in_document_cnt + en_document_cnt;
	
	cerr << "Calculated DF from " << document_cnt << " documents" << endl;
	
	// Calculate TF/DF over the documents we have in memory
	std::vector<DocumentRef> refs(in_document_cnt);
	read_document_refs_from_file(vm["translated-tokens"].as<std::string>(),
								 vm["translated-urls"].as<std::string>(),
								 df, document_cnt, refs.begin());
	
	cerr << "Read " << refs.size() << " documents into memory" << endl;
	
	// Start reading the other set of documents we match against
	// (Note: they are not included in the DF table!)
	
	return score_documents(refs, df,
						   vm["english-tokens"].as<std::string>(),
						   vm["english-urls"].as<std::string>(),
						   threshold,
						   n_threads);
}
