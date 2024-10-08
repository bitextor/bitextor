#include <iostream>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <memory>
#include <vector>
#include <cmath>
#include <boost/program_options.hpp>
#include "util/file_piece.hh"
#include "src/document.h"
#include "src/blocking_queue.h"


using namespace bitextor;
using namespace std;

namespace po = boost::program_options;

struct Line {
	string str;
	size_t n;
};

struct DocumentPair {
	float score;
	size_t in_idx;
	size_t en_idx;
};

struct DocumentNGramScore {
	size_t doc_id;
	float tfidf;
};

constexpr size_t QUEUE_SIZE_PER_THREAD = 32;

constexpr size_t BATCH_SIZE = 512;

/**
 * Utility to start N threads executing fun. Returns a vector with those thread objects.
 */
template <typename T> vector<thread> start(unsigned int n_threads, T fun) {
	vector<thread> threads;
	threads.reserve(n_threads);
	for (unsigned int n = 0; n < n_threads; ++n)
		threads.push_back(thread(fun));
	return threads;
}

/**
 * Utility to stop & join threads. Needs access to the queue to supply it null pointers after which it waits
 * for the workers to stop & join.
 */
template <typename T> void stop(blocking_queue<unique_ptr<T>> &queue, vector<thread> &workers) {
	for (size_t i = 0; i < workers.size(); ++i)
		queue.push(nullptr);

	for (auto &worker : workers)
		worker.join();
}

ostream &operator<<(ostream &out, queue_performance const &performance) {
	return out << "  underflow: " << performance.underflow << '\n'
			   << "   overflow: " << performance.overflow << '\n';
}

void print_score(float score, size_t left_id, size_t right_id)
{
	cout << fixed << setprecision(5)
		 << score
		 << '\t' << left_id
		 << '\t' << right_id
		 << '\n';
}

size_t queue_lines(util::FilePiece &fin, blocking_queue<unique_ptr<vector<Line>>> &queue, size_t skip_rate = 1)
{
	size_t document_count = 0;

	auto it = fin.begin();

	while (it != fin.end()) {
		unique_ptr<vector<Line>> line_batch(new vector<Line>());
		line_batch->reserve(BATCH_SIZE);

		for (size_t i = 0; i < BATCH_SIZE; ++i) {
			if (document_count++ % skip_rate == 0)
				line_batch->push_back({
					.str = string(it->data(), it->size()),
					.n = document_count
				});

			if (++it == fin.end())
				break;
		}

		queue.push(std::move(line_batch));
	}

	return document_count;
}

size_t queue_lines(std::string const &path, blocking_queue<unique_ptr<vector<Line>>> &queue, size_t skip_rate = 1)
{
	util::FilePiece fin(path.c_str());
	return queue_lines(fin, queue, skip_rate);
}

int main(int argc, char *argv[])
{
	unsigned int n_threads = thread::hardware_concurrency();
	
	float threshold = 0.1;
	
	size_t df_sample_rate = 1;
	
	size_t df_table_size = 1 << 20;
	
	size_t ngram_size = 2;

	size_t min_ngram_cnt = 2;

	size_t max_ngram_cnt = 1000;

	bool verbose = false;

	bool print_all = false;
	
	po::positional_options_description arg_desc;
	arg_desc.add("translated-tokens", 1);
	arg_desc.add("english-tokens", 1);
	
	po::options_description generic_desc("Additional options");
	generic_desc.add_options()
		("help", "produce help message")
		("df-sample-rate", po::value<size_t>(&df_sample_rate), "set sample rate to every n-th document (default: 1)")
		("df-table-size", po::value<size_t>(&df_table_size), "set table size (default: 262144)")
		("ngram_size,n", po::value<size_t>(&ngram_size), "ngram size (default: 2)")
		("jobs,j", po::value<unsigned int>(&n_threads), "set number of threads (default: all)")
		("threshold", po::value<float>(&threshold), "set score threshold (default: 0.1)")
		("min_count", po::value<size_t>(&min_ngram_cnt), "minimal number of documents an ngram can appear in to be included in DF (default: 2)")
		("max_count", po::value<size_t>(&max_ngram_cnt), "maximum number of documents for ngram to to appear in (default: 1000)")
		("all", po::bool_switch(&print_all), "print all scores, not only the best pairs")
		("verbose,v", po::bool_switch(&verbose), "show additional output");
	
	po::options_description hidden_desc("Hidden options");
	hidden_desc.add_options()
		("translated-tokens", po::value<string>(), "set input filename")
		("english-tokens", po::value<string>(), "set input filename");

	po::options_description opt_desc;
	opt_desc.add(generic_desc).add(hidden_desc);

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
			 << generic_desc << endl;
		return 1;
	}

	unsigned int n_sample_threads = n_threads;

	unsigned int n_load_threads = n_threads;

	// Note: I've tried many heuristics for the number of reading threads, but
	// my conclusion was that I either have too few and the scoring threads are
	// waiting, or the queue is filled and the reading threads are blocking
	// anyway. On desktop (macOS) just using maximum threads everywhere was
	// always the fastest.
	unsigned int n_read_threads = n_threads;

	unsigned int n_score_threads = n_threads;
	
	// Calculate the document frequency for terms. Starts a couple of threads
	// that parse documents and keep a local hash table for counting. At the
	// end these tables are merged into df.
	HashMap<NGram,size_t> df(df_table_size);
	size_t in_document_cnt, en_document_cnt, document_cnt;

	{
		mutex df_mutex;
		blocking_queue<unique_ptr<vector<Line>>> queue(n_sample_threads * QUEUE_SIZE_PER_THREAD);
		vector<thread> workers(start(n_sample_threads, [&queue, &df, &df_mutex, &ngram_size, &df_table_size, &df_sample_rate]() {
			HashMap<NGram, size_t> local_df(df_table_size);

			while (true) {
				unique_ptr<vector<Line>> line_batch(queue.pop());

				if (!line_batch)
					break;

				for (Line const &line : *line_batch) {
					Document document;
					ReadDocument(line.str, document, ngram_size);
					for (auto const &entry : document.vocab)
						local_df[entry.first] += 1; // Count once every document
				}
			}

			// Merge the local DF into the global one. Multiply by df_sample_rate
			// to compensate for reading only nth part of the whole collection.
			{
				unique_lock<mutex> lock(df_mutex);
				df.add(local_df, df_sample_rate);
			}
		}));

		// We'll use in_document_cnt later to reserve some space for the documents
		// we want to keep in memory. (Also this line is the whole reason the
		// worker management + reading isn't wrapped in a single function: I
		// want to re-use the same workers for two files.)
		en_document_cnt = queue_lines(vm["english-tokens"].as<std::string>(), queue, df_sample_rate);
		in_document_cnt = queue_lines(vm["translated-tokens"].as<std::string>(), queue, df_sample_rate);
		document_cnt = in_document_cnt + en_document_cnt;

		stop(queue, workers);

		if (verbose)
			cerr << "Calculated DF from " << document_cnt / df_sample_rate << " documents" << endl;

		if (verbose)
			cerr << "DF queue performance:\n" << queue.performance();
	}

	// Read translated documents & pre-calculate TF/DF for each of these documents
	unordered_map<NGram, vector<DocumentNGramScore>> ref_index;
	
	{
		mutex ref_index_mutex;

		blocking_queue<unique_ptr<vector<Line>>> queue(n_load_threads * QUEUE_SIZE_PER_THREAD);
		vector<thread> workers(start(n_load_threads, [&queue, &ref_index, &ref_index_mutex, &df, &max_ngram_cnt, &document_cnt, &ngram_size]() {
			unordered_map<NGram, vector<DocumentNGramScore>> local_ref_index;

			while (true) {
				unique_ptr<vector<Line>> line_batch(queue.pop());

				if (!line_batch)
					break;

				for (Line const &line : *line_batch) {
					Document doc{.id = line.n, .vocab = {}};
					ReadDocument(line.str, doc, ngram_size);

					// Note that each worker writes to a different line in the refs
					// vector and the vector has been initialized with enough lines
					// so there should be no concurrency issue.
					// DF is accessed read-only. N starts counting at 1.
					DocumentRef ref;
					calculate_tfidf(doc, ref, document_cnt, df, max_ngram_cnt);

					for (auto const &entry : ref.wordvec) {
						local_ref_index[entry.hash].push_back(DocumentNGramScore{
							.doc_id = line.n,
							.tfidf = entry.tfidf
						});
					}
				}
			}

			{
				// Merge the local index we built into the global one
				unique_lock<mutex> lock(ref_index_mutex);
				for (auto &entry : local_ref_index) {
					auto &dest = ref_index[entry.first];

					// Minor optimisation: copy the fewest elements possible
					if (dest.size() < entry.second.size())
						swap(dest, entry.second);

					dest.reserve(dest.size() + entry.second.size());
					
					move(entry.second.begin(), entry.second.end(), back_inserter(dest));
				}
			}
		}));

		size_t refs_cnt = queue_lines(vm["translated-tokens"].as<std::string>(), queue);

		UTIL_THROW_IF(refs_cnt != in_document_cnt, util::Exception, "Line count changed"
			<< " from " << in_document_cnt << " to " << refs_cnt
			<< " while reading " << vm["translated-tokens"].as<std::string>() 
			<< " in a second pass.");
		
		stop(queue, workers);

		if (verbose)
			cerr << "Read " << refs_cnt << " documents into memory" << endl;

		if (verbose)
			cerr << "Load queue performance:\n" << queue.performance();
	}

	// Start reading the other set of documents we match against and do the matching.
	{
		blocking_queue<unique_ptr<vector<Line>>> read_queue(n_read_threads * QUEUE_SIZE_PER_THREAD);

		blocking_queue<unique_ptr<vector<DocumentRef>>> score_queue(n_score_threads * QUEUE_SIZE_PER_THREAD);

		vector<thread> read_workers(start(n_read_threads, [&read_queue, &score_queue, &document_cnt, &df, &max_ngram_cnt, &ngram_size]() {
			while (true) {
				unique_ptr<vector<Line>> line_batch(read_queue.pop());

				// Empty pointer is poison
				if (!line_batch)
					break;

				unique_ptr<vector<DocumentRef>> ref_batch(new vector<DocumentRef>());
				ref_batch->reserve(line_batch->size());
			
				for (Line const &line : *line_batch) {
					Document doc{.id = line.n, .vocab = {}};
					ReadDocument(line.str, doc, ngram_size);

					ref_batch->emplace_back();
					calculate_tfidf(doc, ref_batch->back(), document_cnt, df, max_ngram_cnt);
				}

				score_queue.push(std::move(ref_batch));
			}
		}));

		// Function used to report the score. Implementation depends on whether
		// we are doing print_all or not. Mutex is necessary for both cases,
		// either for writing to top_scores or for printing to stdout.
		function<void (float, size_t in_ref, size_t en_ref)> mark_score;
		mutex mark_score_mutex;

		// Scores for all pairs (that meet the threshold). Only used with 
		vector<DocumentPair> scored_pairs;

		if (!print_all) {
			mark_score = [&scored_pairs, &mark_score_mutex] (float score, size_t in_ref, size_t en_ref) {
				unique_lock<mutex> lock(mark_score_mutex);
				scored_pairs.push_back({score, in_ref, en_ref});
			};
		} else {
			mark_score = [&mark_score_mutex](float score, size_t in_ref, size_t en_ref) {
				unique_lock<mutex> lock(mark_score_mutex);
				print_score(score, in_ref, en_ref);
			};

			// Print output header
			cout << "mt_doc_aligner_score\tidx_translated\tidx_trg" << endl;
		}

		vector<thread> score_workers(start(n_score_threads, [&score_queue, &ref_index, &threshold, &mark_score]() {
			while (true) {
				unique_ptr<vector<DocumentRef>> doc_ref_batch(score_queue.pop());

				if (!doc_ref_batch)
					break;

				for (auto &doc_ref : *doc_ref_batch) {
					unordered_map<size_t, float> ref_scores;
					
					for (auto const &word_score : doc_ref.wordvec) {
						// Search ngram hash (uint64_t) in ref_index
						auto it = ref_index.find(word_score.hash);
						
						if (it == ref_index.end())
							continue;
						
						for (auto const &ref_score : it->second)
							ref_scores[ref_score.doc_id] += word_score.tfidf * ref_score.tfidf;
					}

					for (auto const &ref : ref_scores)
						if (ref.second >= threshold)
							mark_score(ref.second, ref.first, doc_ref.id);
				}
			}
		}));

		size_t read_cnt = queue_lines(vm["english-tokens"].as<std::string>(), read_queue);

		UTIL_THROW_IF(read_cnt != en_document_cnt, util::Exception, "Line count changed"
			<< " from " << en_document_cnt << " to " << read_cnt
			<< " while reading " << vm["english-tokens"].as<std::string>() 
			<< " in a second pass.");

		// Tell all workers there is nothing left and wait for them to stop.
		stop(read_queue, read_workers);
		stop(score_queue, score_workers);

		if (!print_all) {
			// Sort scores, best on top. Also sort on other properties to make
			// it a consistent order, c.f. not depending on the processing order.
			sort(scored_pairs.begin(), scored_pairs.end(), [](DocumentPair const &a, DocumentPair const &b) {
				if (a.score != b.score)
					return a.score > b.score;

				if (a.in_idx != b.in_idx)
					return a.in_idx > b.in_idx;

				return a.en_idx > b.en_idx;
			});

			// Keep track of which documents have already been assigned
			vector<bool> in_seen(in_document_cnt);
			vector<bool> en_seen(en_document_cnt);

			// Also keep a quick tally on whether we've printed scores for
			// every document, so we don't keep searching while in_seen or
			// en_seen is completely filled.
			size_t cnt = 0;
			size_t document_cnt = min(in_document_cnt, en_document_cnt);

			// Print output header
			cout << "mt_doc_aligner_score\tidx_translated\tidx_trg" << endl;

			// For each pair (with score, sorted from good to bad)
			for (DocumentPair const &pair : scored_pairs) {
				// If either of the documents has already been printed, skip it.
				if (in_seen[pair.in_idx - 1] || en_seen[pair.en_idx - 1])
					continue;

				print_score(pair.score, pair.in_idx, pair.en_idx);
				in_seen[pair.in_idx - 1] = true;
				en_seen[pair.en_idx - 1] = true;

				if (++cnt == document_cnt)
					break;
			}
		}

		if (verbose)
			cerr << "Read queue performance (Note: blocks when score queue fills up):\n" << read_queue.performance()
				 << "Score queue performance:\n" << score_queue.performance();
	}

	return 0;
}
