/** Induces word categories
 *  By Jon Dehdari, 2014-2016
 *  Usage: ./clustercat [options] < corpus.tok.txt > classes.tsv
**/

#include <limits.h>				// UCHAR_MAX, UINT_MAX
#include <float.h>				// DBL_MAX, etc.
#include <math.h>				// isnan()
#include <time.h>				// clock_t, clock(), CLOCKS_PER_SEC
#include <stdbool.h>
#include <locale.h>				// OPTIONAL!  Comment-out on non-Posix machines, and the function setlocale() in the first line of main()

#include "clustercat.h"						// Model importing/exporting functions
#include "clustercat-array.h"				// which_maxf()
#include "clustercat-data.h"
#include "clustercat-cluster.h"				// cluster()
#include "clustercat-dbg.h"					// for printing out various complex data structures
#include "clustercat-import-class-file.h"	// import_class_file()
#include "clustercat-io.h"					// process_input()
#include "clustercat-math.h"				// perplexity(), powi()

#define USAGE_LEN 10000
#define LOG2ADD(a,b) (log2(a) + log2(1 + (b) / (a) ))

// Declarations
void get_usage_string(char * restrict usage_string, int usage_len);
void parse_cmd_args(const int argc, char **argv, char * restrict usage, struct cmd_args *cmd_args);
char * restrict class_algo           = NULL;
char * restrict in_train_file_string = NULL;
char * restrict out_file_string      = NULL;
char * restrict initial_class_file   = NULL;

struct_map_word *word_map = NULL; // Must initialize to NULL
struct_map_bigram *initial_bigram_map = NULL; // Must initialize to NULL
struct_map_bigram *new_bigram_map     = NULL; // Must initialize to NULL
struct_map_bigram *new_bigram_map_rev = NULL; // Must initialize to NULL
char usage[USAGE_LEN];
size_t memusage = 0;


// Defaults
struct cmd_args cmd_args = {
	.class_algo         = EXCHANGE,
	.class_offset       = 0,
	.forward_lambda     = 0.55,
	.min_count          = 3, // or max(2, floor(N^0.14 - 7))
	.max_array          = 2,
	.ngram_input        = false,
	.num_threads        = 8,
	.num_classes        = 0,
	.print_freqs        = false,
	.print_word_vectors = NO_VEC,
	.refine             = 2,
	.rev_alternate      = 3,
	.tune_cycles        = 15,
	.unidirectional     = false,
	.verbose            = 0,
};



int main(int argc, char **argv) {
	setlocale(LC_ALL, ""); // Comment-out on non-Posix systems
	clock_t time_start = clock();
	time_t time_t_start;
	time(&time_t_start);
	argv_0_basename = basename(argv[0]);
	get_usage_string(usage, USAGE_LEN); // This is a big scary string, so build it elsewhere

	//printf("sizeof(cmd_args)=%zd\n", sizeof(cmd_args));
	parse_cmd_args(argc, argv, usage, &cmd_args);

	if (cmd_args.class_algo == EXCHANGE || cmd_args.class_algo == EXCHANGE_BROWN)
		memusage += sizeof(float) * ENTROPY_TERMS_MAX; // We'll build the precomputed entropy terms after reporting memusage

	struct_model_metadata global_metadata;

	// The list of unique words should always include <s>, unknown word, and </s>
	map_update_count(&word_map, UNKNOWN_WORD, 0, 0); // Should always be first
	map_update_count(&word_map, "<s>", 0, 1);
	map_update_count(&word_map, "</s>", 0, 2);

	// Open input
	FILE *in_train_file = stdin;
	if (in_train_file_string)
		in_train_file = fopen(in_train_file_string, "r");
	if (in_train_file == NULL) {
		fprintf(stderr, "%s: Error: Unable to open input file  %s\n", argv_0_basename, in_train_file_string); fflush(stderr);
		exit(15);
	}

	// Process input sentences
	size_t input_memusage = 0;
	const struct_model_metadata input_model_metadata = process_input(cmd_args, in_train_file, &word_map, &initial_bigram_map, &input_memusage);
	memusage += input_memusage;
	fclose(in_train_file);

	clock_t time_input_processed = clock();
	if (cmd_args.verbose >= -1) {
		fprintf(stderr, "%s: Corpus processed in %'.2f CPU secs. %'lu lines, %'u types, %'lu tokens, current memusage: %'.1fMB\n", argv_0_basename, (double)(time_input_processed - time_start)/CLOCKS_PER_SEC, input_model_metadata.line_count, input_model_metadata.type_count, input_model_metadata.token_count, (double)memusage / 1048576); fflush(stderr);
	}

	global_metadata.token_count = input_model_metadata.token_count;
	global_metadata.type_count  = map_count(&word_map);

	// Filter out infrequent words, reassign word_id's, and build a mapping from old word_id's to new word_id's
	sort_by_count(&word_map);
	word_id_t * restrict word_id_remap = calloc(sizeof(word_id_t), input_model_metadata.type_count);
	get_ids(&word_map, word_id_remap);
	word_id_t number_of_deleted_words = filter_infrequent_words(cmd_args, &global_metadata, &word_map, word_id_remap);

	// Get list of unique words
	char * * restrict word_list = (char **)malloc(sizeof(char*) * global_metadata.type_count);
	memusage += sizeof(char*) * global_metadata.type_count;
	reassign_word_ids(&word_map, word_list, word_id_remap);
	get_keys(&word_map, word_list);
	sort_by_id(&word_map);


	// Check or set number of classes
	if (cmd_args.num_classes >= global_metadata.type_count) { // User manually set number of classes is too low
		fprintf(stderr, "%s: Error: Number of classes (%u) is not less than vocabulary size (%u).  Decrease the value of --classes\n", argv_0_basename, cmd_args.num_classes, global_metadata.type_count); fflush(stderr);
		exit(3);
	} else if (cmd_args.num_classes == 0) { // User did not manually set number of classes at all
		cmd_args.num_classes = (wclass_t) (sqrt(global_metadata.type_count) * 1.2);
	}

	// Build array of word_counts
	word_count_t * restrict word_counts = malloc(sizeof(word_count_t) * global_metadata.type_count);
	memusage += sizeof(word_count_t) * global_metadata.type_count;
	build_word_count_array(&word_map, word_list, word_counts, global_metadata.type_count);

	// Initialize clusters, and possibly read-in external class file
	wclass_t * restrict word2class = malloc(sizeof(wclass_t) * global_metadata.type_count);
	memusage += sizeof(wclass_t) * global_metadata.type_count;
	init_clusters(cmd_args, global_metadata.type_count, word2class, word_counts, word_list);
	if (initial_class_file != NULL)
		import_class_file(&word_map, word2class, initial_class_file, cmd_args.num_classes); // Overwrite subset of word mappings, from user-provided initial_class_file

	// Remap word_id's in initial_bigram_map
	remap_and_rev_bigram_map(&initial_bigram_map, &new_bigram_map, &new_bigram_map_rev, word_id_remap, map_find_id(&word_map, UNKNOWN_WORD, -1));
	global_metadata.start_sent_id = map_find_id(&word_map, "<s>", -1);; // need this for tallying emission probs
	global_metadata.end_sent_id   = map_find_id(&word_map, "</s>", -1);; // need this for tallying emission probs
	global_metadata.line_count    = map_find_count(&word_map, "</s>"); // Used for calculating perplexity

	if (global_metadata.line_count == 0) {
		fprintf(stderr, "%s: Warning: Number of lines is 0.  Include <s> and </s> in your ngram counts, or perplexity values will be unreliable.\n", argv_0_basename); fflush(stderr);
	}

	//printf("init_bigram_map hash_count=%u\n", HASH_COUNT(initial_bigram_map)); fflush(stdout);
	//printf("new_bigram_map hash_count=%u\n", HASH_COUNT(new_bigram_map)); fflush(stdout);
	free(word_id_remap);
	memusage -= sizeof(word_id_t) * input_model_metadata.type_count;
	delete_all(&word_map); // static
	delete_all_bigram(&initial_bigram_map); // static
	memusage -= input_memusage;

	// Initialize and set word bigram listing
	clock_t time_bigram_start = clock();
	size_t bigram_memusage = 0; size_t bigram_rev_memusage = 0;
	struct_word_bigram_entry * restrict word_bigrams = NULL;
	struct_word_bigram_entry * restrict word_bigrams_rev = NULL;

	if (cmd_args.verbose >= -1) {
		fprintf(stderr, "%s: Word bigram listing ... ", argv_0_basename); fflush(stderr);
	}

	#pragma omp parallel sections // Both bigram listing and reverse bigram listing can be done in parallel
	{
		#pragma omp section
		{
			//sort_bigrams(&new_bigram_map); // speeds things up later
			word_bigrams = calloc(global_metadata.type_count, sizeof(struct_word_bigram_entry));
			memusage += sizeof(struct_word_bigram_entry) * global_metadata.type_count;
			bigram_memusage = set_bigram_counts(word_bigrams, new_bigram_map);
			// Copy entries in word_counts to struct_word_bigram_entry.headword_count since that struct entry is already loaded when clustering
			for (word_id_t word = 0; word < global_metadata.type_count; word++)
				word_bigrams[word].headword_count = word_counts[word];
		}

		// Initialize and set *reverse* word bigram listing
		#pragma omp section
		{
			if (cmd_args.rev_alternate) { // Don't bother building this if it won't be used
				//sort_bigrams(&new_bigram_map_rev); // speeds things up later
				word_bigrams_rev = calloc(global_metadata.type_count, sizeof(struct_word_bigram_entry));
				memusage += sizeof(struct_word_bigram_entry) * global_metadata.type_count;
				bigram_rev_memusage = set_bigram_counts(word_bigrams_rev, new_bigram_map_rev);
				// Copy entries in word_counts to struct_word_bigram_entry.headword_count since that struct entry is already loaded when clustering
				for (word_id_t word = 0; word < global_metadata.type_count; word++)
					word_bigrams_rev[word].headword_count = word_counts[word];
			}
		}
	}

	delete_all_bigram(&new_bigram_map);
	delete_all_bigram(&new_bigram_map_rev);
	memusage += bigram_memusage + bigram_rev_memusage;
	clock_t time_bigram_end = clock();
	if (cmd_args.verbose >= -1) {
		fprintf(stderr, "in %'.2f CPU secs.  Bigram memusage: %'.1f MB\n", (double)(time_bigram_end - time_bigram_start)/CLOCKS_PER_SEC, (bigram_memusage + bigram_rev_memusage)/(double)1048576); fflush(stderr);
	}

	//print_word_bigrams(global_metadata, word_bigrams, word_list);

	// Build <v,c> counts, which consists of a word followed by a given class
	word_class_count_t * restrict word_class_counts = calloc(1 + cmd_args.num_classes * global_metadata.type_count , sizeof(word_class_count_t));
	if (word_class_counts == NULL) {
		fprintf(stderr,  "%s: Error: Unable to allocate enough memory for <v,c>.  %'.1f MB needed.  Maybe increase --min-count\n", argv_0_basename, ((cmd_args.num_classes * global_metadata.type_count * sizeof(word_class_count_t)) / (double)1048576 )); fflush(stderr);
		exit(13);
	}
	memusage += cmd_args.num_classes * global_metadata.type_count * sizeof(word_class_count_t);
	fprintf(stderr, "%s: Allocating %'.1f MB for word_class_counts: num_classes=%u x type_count=%u x sizeof(w-cl-count_t)=%zu\n", argv_0_basename, (double)(cmd_args.num_classes * global_metadata.type_count * sizeof(word_class_count_t)) / 1048576 , cmd_args.num_classes, global_metadata.type_count, sizeof(word_class_count_t)); fflush(stderr);
	build_word_class_counts(cmd_args, word_class_counts, word2class, word_bigrams, global_metadata.type_count/*, word_list*/);
	//print_word_class_counts(cmd_args, global_metadata, word_class_counts);

	// Build reverse: <c,v> counts: class followed by word.  This and the normal one are both pretty fast, so no need to parallelize this
	word_class_count_t * restrict word_class_rev_counts = NULL;
	if (cmd_args.rev_alternate) { // Don't bother building this if it won't be used
		word_class_rev_counts = calloc(1 + cmd_args.num_classes * global_metadata.type_count , sizeof(word_class_count_t));
		if (word_class_rev_counts == NULL) {
			fprintf(stderr,  "%s: Warning: Unable to allocate enough memory for <v,c>.  %'.1f MB needed.  Falling back to --rev-alternate 0\n", argv_0_basename, ((cmd_args.num_classes * global_metadata.type_count * sizeof(word_class_count_t)) / (double)1048576 )); fflush(stderr);
			cmd_args.rev_alternate = 0;
		} else {
			memusage += cmd_args.num_classes * global_metadata.type_count * sizeof(word_class_count_t);
			fprintf(stderr, "%s: Allocating %'.1f MB for word_class_rev_counts: num_classes=%u x type_count=%u x sizeof(w-cl-count_t)=%zu\n", argv_0_basename, (double)(cmd_args.num_classes * global_metadata.type_count * sizeof(word_class_count_t)) / 1048576 , cmd_args.num_classes, global_metadata.type_count, sizeof(word_class_count_t)); fflush(stderr);
			build_word_class_counts(cmd_args, word_class_rev_counts, word2class, word_bigrams_rev, global_metadata.type_count/*, word_list*/);
		}

	}

	// Calculate memusage for count_arrays
	for (unsigned char i = 1; i <= cmd_args.max_array; i++) {
		memusage += 2 * (powi(cmd_args.num_classes, i) * sizeof(wclass_count_t));
		//printf("11 memusage += %zu (now=%zu) count_arrays\n", 2 * (powi(cmd_args.num_classes, i) * sizeof(wclass_count_t)), memusage); fflush(stdout);
	}

	clock_t time_model_built = clock();
	if (cmd_args.verbose >= -1) {
		fprintf(stderr, "%s: Finished loading %'lu tokens and %'u types (%'u filtered) from %'lu lines in %'.2f CPU secs\n", argv_0_basename, global_metadata.token_count, global_metadata.type_count, number_of_deleted_words, global_metadata.line_count, (double)(time_model_built - time_start)/CLOCKS_PER_SEC); fflush(stderr);
	}
	if (cmd_args.verbose >= -1) {
		fprintf(stderr, "%s: Approximate memory usage at clustering: %'.1fMB\n", argv_0_basename, (double)memusage / 1048576); fflush(stderr);
	}

	cluster(cmd_args, global_metadata, word_counts, word_list, word2class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts);

	// Now print the final word2class mapping
	if (cmd_args.verbose >= 0) {
		FILE *out_file = stdout;
		if (out_file_string)
			out_file = fopen(out_file_string, "w");
		if (out_file == NULL) {
			fprintf(stderr, "%s: Error: Unable to open output file  %s\n", argv_0_basename, out_file_string); fflush(stderr);
			exit(16);
		}
		if (cmd_args.class_algo == EXCHANGE && (!cmd_args.print_word_vectors)) {
			print_words_and_classes(out_file, global_metadata.type_count, word_list, word_counts, word2class, (int)cmd_args.class_offset, cmd_args.print_freqs);
		} else if (cmd_args.class_algo == EXCHANGE && cmd_args.print_word_vectors) {
			print_words_and_vectors(out_file, cmd_args, global_metadata, word_list, word2class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts);
		}
		fclose(out_file);
	}

	clock_t time_clustered = clock();
	time_t time_t_end;
	time(&time_t_end);
	double time_secs_total = difftime(time_t_end, time_t_start);
	if (cmd_args.verbose >= -1)
		fprintf(stderr, "%s: Finished clustering in %'.2f CPU seconds.  Total wall clock time was about %lim %lis\n", argv_0_basename, (double)(time_clustered - time_model_built)/CLOCKS_PER_SEC, (long)time_secs_total/60, ((long)time_secs_total % 60)  );

	free(word2class);
	free(word_bigrams);
	free(word_list);
	free(word_counts);
	exit(0);
}


void get_usage_string(char * restrict usage_string, int usage_len) {

	snprintf(usage_string, usage_len, "ClusterCat  (c) 2014-2016 Jon Dehdari - LGPL v3 or Mozilla Public License v2\n\
\n\
Usage:    clustercat [options] < corpus.tok.txt > classes.tsv \n\
\n\
Function: Induces word categories from plaintext\n\
\n\
Options:\n\
 -c, --classes <hu>       Set number of word classes (default: 1.2 * square root of vocabulary size)\n\
     --class-file <file>  Initialize exchange word classes from an existing clustering tsv file (default: pseudo-random initialization\n\
                          for exchange). If you use this option, you probably can set --tune-cycles to 3 or so\n\
     --class-offset <c>   Print final word classes starting at a given number (default: %d)\n\
     --forward-lambda <f> Set interpolation weight for forward bigram class model, in range of [0,1] (default: %g)\n\
 -h, --help               Print this usage\n\
     --in <file>          Specify input training file (default: stdin)\n\
     --ngram-input        Input is a listing of n-grams and their counts. Otherwise input is a normal corpus\n\
     --min-count <hu>     Minimum count of entries in training set to consider (default: %d occurrences)\n\
     --max-array <c>      Set maximum order of n-grams for which to use an array instead of a sparse hash map (default: %d-grams)\n\
     --out <file>         Specify output file (default: stdout)\n\
     --print-freqs        Print word frequencies after words and classes in final clustering output (useful for visualization)\n\
 -q, --quiet              Print less output.  Use additional -q for even less output\n\
     --refine <c>         Set initial class refinement value (c==0 -> no refinement; otherwise 2^n.  Default:c==2 -> 4 initial clusters)\n\
     --rev-alternate <u>  How often to alternate using reverse predictive exchange. 0==never, 1==after every normal cycle (default: %u)\n\
 -j, --threads <hu>       Set number of threads to run simultaneously (default: %d threads)\n\
     --tune-cycles <hu>   Set max number of cycles to tune on (default: %d cycles)\n\
     --unidirectional     Disable simultaneous bidirectional predictive exchange. Results in faster cycles, but slower & worse convergence\n\
                          If you want to do basic predictive exchange, use:  --rev-alternate 0 --unidirectional\n\
 -v, --verbose            Print additional info to stderr.  Use additional -v for more verbosity\n\
     --word-vectors <s>   Print word vectors (a.k.a. word embeddings) instead of discrete classes.\n\
                          Specify <s> as either 'text' or 'binary'.  The binary format is compatible with word2vec\n\
\n\
", cmd_args.class_offset, cmd_args.forward_lambda, cmd_args.min_count, cmd_args.max_array, cmd_args.rev_alternate, cmd_args.num_threads, cmd_args.tune_cycles);
}
//     --class-algo <s>     Set class-induction algorithm {brown,exchange,exchange-then-brown} (default: exchange)\n\
// -o, --order <i>          Maximum n-gram order in training set to consider (default: %d-grams)\n\
// -w, --weights 'f f ...'  Set class interpolation weights for: 3-gram, 2-gram, 1-gram, rev 2-gram, rev 3-gram. (default: %s)\n\

void parse_cmd_args(int argc, char **argv, char * restrict usage, struct cmd_args *cmd_args) {
	for (int arg_i = 0; arg_i < argc; arg_i++) // Print command-line invocation, for reproducibility
		if (cmd_args->verbose >= -1) {
			fprintf(stderr, "%s ", argv[arg_i]); fflush(stderr);
		}
	if (cmd_args->verbose >= -1) {
		fprintf(stderr, "\n"); fflush(stderr);
	}

	for (int arg_i = 1; arg_i < argc; arg_i++) {
		if (!(strcmp(argv[arg_i], "-h") && strcmp(argv[arg_i], "--help"))) {
			printf("%s", usage);
			exit(0);
		} else if (!strcmp(argv[arg_i], "--class-algo")) {
			char * restrict class_algo_string = argv[arg_i+1];
			arg_i++;
			if (!strcmp(class_algo_string, "brown"))
				cmd_args->class_algo = BROWN;
			else if (!strcmp(class_algo_string, "exchange"))
				cmd_args->class_algo = EXCHANGE;
			else if (!strcmp(class_algo_string, "exchange-then-brown"))
				cmd_args->class_algo = EXCHANGE_BROWN;
			else { printf("%s", usage); exit(1); }
		} else if (!strcmp(argv[arg_i], "--class-file")) {
			initial_class_file = argv[arg_i+1];
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--class-offset")) {
			cmd_args->class_offset = (signed char)atoi(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--forward-lambda")) {
			cmd_args->forward_lambda = (float)atof(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--in")) {
			in_train_file_string = argv[arg_i+1];
			arg_i++;
		} else if (!(strcmp(argv[arg_i], "-j") && strcmp(argv[arg_i], "--threads") && strcmp(argv[arg_i], "--jobs"))) {
			cmd_args->num_threads = (unsigned int) atol(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--min-count")) {
			cmd_args->min_count = (unsigned int) atol(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--max-array")) {
			cmd_args->max_array = (unsigned char) atol(argv[arg_i+1]);
			if ((cmd_args->max_array) < 1 || (cmd_args->max_array > 3)) {
				printf("%s: --max-array value should be between 1-3\n", argv_0_basename);
				fflush(stderr);
				exit(10);
			}
			arg_i++;
		} else if (!(strcmp(argv[arg_i], "--ngram-input"))) {
			cmd_args->ngram_input = true;
		} else if (!(strcmp(argv[arg_i], "-c") && strcmp(argv[arg_i], "-n") && strcmp(argv[arg_i], "--classes") && strcmp(argv[arg_i], "--num-classes"))) {
			cmd_args->num_classes = (wclass_t) atol(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--out")) {
			out_file_string = argv[arg_i+1];
			arg_i++;
		} else if (!(strcmp(argv[arg_i], "--print-freqs"))) {
			cmd_args->print_freqs = true;
		} else if (!(strcmp(argv[arg_i], "-q") && strcmp(argv[arg_i], "--quiet"))) {
			cmd_args->verbose--;
		} else if (!(strcmp(argv[arg_i], "--refine"))) {
			cmd_args->refine = (unsigned char) atol(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--rev-alternate")) {
			cmd_args->rev_alternate = (unsigned char) atoi(argv[arg_i+1]);
			arg_i++;
		} else if (!strcmp(argv[arg_i], "--tune-cycles")) {
			cmd_args->tune_cycles = (unsigned short) atol(argv[arg_i+1]);
			arg_i++;
		} else if (!(strcmp(argv[arg_i], "--unidirectional"))) {
			cmd_args->unidirectional = true;
		} else if (!(strcmp(argv[arg_i], "-v") && strcmp(argv[arg_i], "--verbose"))) {
			cmd_args->verbose++;
		} else if (!(strcmp(argv[arg_i], "--word-vectors"))) {
			char * restrict print_word_vectors_string = argv[arg_i+1];
			arg_i++;
			if (!strcmp(print_word_vectors_string, "text"))
				cmd_args->print_word_vectors = TEXT_VEC;
			else if (!strcmp(print_word_vectors_string, "binary"))
				cmd_args->print_word_vectors = BINARY_VEC;
			else { printf("Error: Please specify either 'text' or 'binary' after the --word-vectors flag.\n\n%s", usage); exit(1); }
		} else if (!strncmp(argv[arg_i], "-", 1)) { // Unknown flag
			printf("%s: Unknown command-line argument: %s\n\n", argv_0_basename, argv[arg_i]);
			printf("%s", usage); fflush(stderr);
			exit(2);
		}
	}
}

void build_word_count_array(struct_map_word **word_map, char * restrict word_list[const], word_count_t word_counts[restrict], const word_id_t type_count) {
	for (word_id_t i = 0; i < type_count; i++) {
		word_counts[i] = map_find_count(word_map, word_list[i]);
	}
}

void populate_word_ids(struct_map_word **word_map, char * restrict word_list[const], const word_id_t type_count) {
	for (word_id_t i = 0; i < type_count; i++) {
		map_set_word_id(word_map, word_list[i], i);
	}
}

void reassign_word_ids(struct_map_word **word_map, char * restrict word_list[restrict], word_id_t * restrict word_id_remap) {
	sort_by_count(word_map);
	struct_map_word *entry, *tmp;
	word_id_t i = 0;

	HASH_ITER(hh, *word_map, entry, tmp) {
		const word_id_t word_id = entry->word_id;
		char * word = entry->key;
		word_id_remap[word_id] = i; // set remap
		word_list[i] = entry->key;
		//printf("reassigning w=%s %u -> %u; count=%u\n", entry->key, word_id, i, entry->count); fflush(stdout);
		map_set_word_id(word_map, word, i); // reset word_id in word_map
		i++;
	}
}

word_id_t filter_infrequent_words(const struct cmd_args cmd_args, struct_model_metadata * restrict model_metadata, struct_map_word ** word_map, word_id_t * restrict word_id_remap) { // word_map must already be sorted by word frequency!

	unsigned long number_of_deleted_words = 0;
	unsigned long vocab_size = model_metadata->type_count; // Save this to separate variable since we'll modify model_metadata.type_count later
	// Get keys
	// Iterate over keys
	//   If count of key_i < threshold,
	//     increment count of <unk> by count of key_i,
	//     decrement model_metadata.type_count by one
	//     free & delete entry in map,

	char **local_word_list = (char **)malloc(model_metadata->type_count * sizeof(char*));
	//char * local_word_list[model_metadata->type_count];
	if (vocab_size != get_keys(word_map, local_word_list)) {
		printf("Error: model_metadata->type_count (%lu) != get_keys() (%lu)\n", (long unsigned) vocab_size, (long unsigned) get_keys(word_map, local_word_list) ); fflush(stderr);
		exit(4);
	}

	unsigned long new_id = 0;
	for (unsigned long word_i = 0; word_i < vocab_size; word_i++, new_id++) {
		char * word = local_word_list[word_i];
		//if ((!strncmp(word, UNKNOWN_WORD, MAX_WORD_LEN)) || (!strncmp(word, "<s>", MAX_WORD_LEN)) || (!strncmp(word, "</s>", MAX_WORD_LEN))) { // Deal with <unk>, <s>, and </s>
		//	//new_id--;
		//	continue;
		//}

		unsigned long word_i_count = map_find_count(word_map, word);  // We'll use this a couple times
		if ((word_i_count < cmd_args.min_count) && (strncmp(word, UNKNOWN_WORD, MAX_WORD_LEN)) && (strncmp(word, "<s>", MAX_WORD_LEN)) &&  (strncmp(word, "</s>", MAX_WORD_LEN))) { // Don't delete <unk>
			number_of_deleted_words++;
			if (cmd_args.verbose > 3) {
				printf("Filtering-out word: %s (old id=%lu, new id=0) (%lu < %hu);\tcount(%s)=%lu\n", word, word_i, (unsigned long)word_i_count, cmd_args.min_count, UNKNOWN_WORD, (unsigned long)map_find_count(word_map, UNKNOWN_WORD)); fflush(stdout);
			}
			word_id_remap[map_find_id(word_map, word, (word_id_t) -1)] = (word_id_t) -1; // set value of dud word in remap to temporary unk, which is -1.  This gets changed later
			map_update_count(word_map, UNKNOWN_WORD, word_i_count, 0);
			model_metadata->type_count--;
			struct_map_word *local_s;
			HASH_FIND_STR(*word_map, word, local_s);
			delete_entry(word_map, local_s);
		} else { // Keep word
			//printf("Keeping word: %s (old id=%u, new id=%lu) (%lu >= %hu);\tcount(%s)=%u\n", word, map_find_id(word_map, word, -1), new_id, word_i_count, cmd_args.min_count, UNKNOWN_WORD, map_find_count(word_map, UNKNOWN_WORD)); fflush(stdout);
			//map_set_word_id(word_map, word, new_id); // word_id's 0-2 are reserved for <unk>, <s>, and </s>
			//printf("  Kept word: %s (new map id=%u, new_id=%lu) (%lu >= %hu);\tcount(%s)=%u\n", word, map_find_id(word_map, word, -1), new_id, word_i_count, cmd_args.min_count, UNKNOWN_WORD, map_find_count(word_map, UNKNOWN_WORD)); fflush(stdout);
		}
	}
	//map_set_word_id(word_map, UNKNOWN_WORD, 0); // word_id's 0-2 are reserved for <unk>, <s>, and </s>
	//map_set_word_id(word_map, "<s>", 1); // word_id's 0-2 are reserved for <unk>, <s>, and </s>
	//map_set_word_id(word_map, "</s>", 2); // word_id's 0-2 are reserved for <unk>, <s>, and </s>

	free(local_word_list);
	return number_of_deleted_words;
}

void tally_class_ngram_counts(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const struct_word_bigram_entry word_bigrams[const], const wclass_t word2class[const], count_arrays_t count_arrays) { // Right now it's a drop-in replacement for tally_class_counts_in_store(), but it's not the best way of doing things (eg. for unigram counts, tallying & querying in two separate steps, etc).  So this will need to be modified after getting rid of the sent-store
	for (word_id_t word_id = 0; word_id < model_metadata.type_count; word_id++) {
		const wclass_t headword_class = word2class[word_id];
		count_arrays[0][headword_class] += word_bigrams[word_id].headword_count;
		//printf("tally_class_ngram_counts: word=??, word_id=%u, type_count=%u, headword_class=%hu, headword_count=%u, class_count=%lu\n", word_id, model_metadata.type_count, headword_class, word_bigrams[word_id].headword_count, (unsigned long)count_arrays[0][headword_class]); fflush(stdout);
		for (unsigned int i = 0; i < word_bigrams[word_id].length; i++) {
			const word_id_t prev_word = word_bigrams[word_id].predecessors[i];
			wclass_t prev_class = word2class[prev_word];
			const size_t offset = prev_class + cmd_args.num_classes * headword_class;
			//printf("  tally_class_ngram_counts: prev_word=%u, prev_class=%hu, offset=%zu\n", prev_word, prev_class, offset); fflush(stdout);
			count_arrays[1][offset] += word_bigrams[word_id].bigram_counts[i];
		}
	}
}


void init_clusters(const struct cmd_args cmd_args, word_id_t vocab_size, wclass_t word2class[restrict], const word_count_t word_counts[const], char * word_list[restrict]) {
	register unsigned long word_i = 0;

	if (cmd_args.class_algo == EXCHANGE || cmd_args.class_algo == EXCHANGE_BROWN) { // It doesn't really matter how you initialize word classes in exchange algo.  This assigns words from the word list an incrementing class number from [0,num_classes-1].  So it's a simple pseudo-randomized initialization.
		register wclass_t class = 0; // [0,num_classes-1]
		for (; word_i < vocab_size; word_i++, class++) {
			if (class == cmd_args.num_classes) // reset
				class = 0;
			if (cmd_args.verbose > 3)
				printf("cls=%-4u w_i=%-8lu #(w)=%-8u str(w)=%-20s vocab_size=%u\n", class, word_i, word_counts[word_i], word_list[word_i], vocab_size);
			word2class[word_i] = class;
		}

	} else if (cmd_args.class_algo == BROWN) { // Really simple initialization: one class per word
		for (unsigned long class = 0; word_i < vocab_size; word_i++, class++)
			word2class[word_i] = class;
	}
}

size_t set_bigram_counts(struct_word_bigram_entry * restrict word_bigrams, struct_map_bigram * bigram_map) {

	// Build a hash map of bigrams, since we need random access when traversing the corpus.
	// Then we convert that to an array of linked lists, since we'll need sequential access during the clustering phase of predictive exchange clustering.

	sort_bigrams(&bigram_map);

	register size_t memusage = 0;
	register word_id_t word_2;
	register word_id_t word_2_last = 0;
	register unsigned int length = 0;
	word_id_t * word_buffer     = malloc(sizeof(word_id_t) * MAX_WORD_PREDECESSORS);
	word_bigram_count_t * count_buffer = malloc(sizeof(word_bigram_count_t) * MAX_WORD_PREDECESSORS);

	// Add a dummy entry at the end of the hash map in order to simplify iterating through it, since it must track changes in head words.
	struct_word_bigram dummy = {-1, -1}; // Make sure this bigram is new, so that it's appended to end
	map_update_bigram(&bigram_map, &dummy, 0);

	// Iterate through bigram map to get counts of word_2's, so we know how much to allocate for each predecessor list
	struct_map_bigram *entry, *tmp;
	HASH_ITER(hh, bigram_map, entry, tmp) {
		word_2 = (entry->key).word_2;
		//printf("\n[%u,%u]=%u, w2_last=%u, length=%u\n", (entry->key).word_1, (entry->key).word_2, entry->count, word_2_last, length); fflush(stdout);
		if (word_2 == word_2_last) { // Within successive entry; ie. 2nd entry or greater
			word_buffer[length]  = (entry->key).word_1;
			count_buffer[length] = entry->count;
			if (length < MAX_WORD_PREDECESSORS)
				length++;
			else {
				printf("Error: MAX_WORD_PREDECESSORS exceeded (%lu).  Increase it in clustercat.h and recompile.  Add the -B flag to 'make' to force recompilation.\n", (long unsigned int)MAX_WORD_PREDECESSORS); fflush(stderr);
				exit(14);
			}
		} else { // New entry; process previous entry
			word_bigrams[word_2_last].length = length;
			word_bigrams[word_2_last].predecessors  = malloc(length * sizeof(word_id_t));
			memcpy(word_bigrams[word_2_last].predecessors,  word_buffer, length * sizeof(word_id_t));
			memusage += length * sizeof(word_id_t);
			word_bigrams[word_2_last].bigram_counts = malloc(length * sizeof(word_bigram_count_t));
			memcpy(word_bigrams[word_2_last].bigram_counts, count_buffer , length * sizeof(word_bigram_count_t));
			memusage += length * sizeof(word_bigram_count_t);
			//printf("word_2_last=%u, length=%u word_1s: ", word_2_last, length);
			//for (unsigned int i = 0; i < length; i++) {
			//	printf("<%u,%u> ", word_bigrams[word_2_last].predecessors[i], word_bigrams[word_2_last].bigram_counts[i]);
			//}
			//printf("\n");

			word_2_last = word_2;
			word_buffer[0]  = (entry->key).word_1;
			count_buffer[0] = entry->count;
			length = 1;
		}
	}

	free(word_buffer);
	free(count_buffer);
	//delete_all_bigram(&map_bigram);

	return memusage;
}

void build_word_class_counts(const struct cmd_args cmd_args, word_class_count_t * restrict word_class_counts, const wclass_t word2class[const], const struct_word_bigram_entry * const word_bigrams, const word_id_t type_count/*, char ** restrict word_list*/) {
	//long sum = 0;
	// set <v,c> counts
	for (word_id_t word = 0; word < type_count; word++) {
		for (unsigned int i = 0; i < word_bigrams[word].length; i++) {
			word_id_t prev_word = word_bigrams[word].predecessors[i];
			const wclass_t class_i = word2class[word];
			word_class_counts[prev_word * cmd_args.num_classes + class_i] += word_bigrams[word].bigram_counts[i];
			//printf("i=%hu, <%s,%s>=<%u,%u>, <v,c>=<%u,%u>, num_classes=%u, offset=%u (%u * %u + %u), orig_val=%u\n", i, word_list[prev_word], word_list[word], prev_word, word, prev_word, class_i, cmd_args.num_classes, prev_word * cmd_args.num_classes + class_i, prev_word, cmd_args.num_classes, class_i, word_class_counts[prev_word * cmd_args.num_classes + class_i]); fflush(stdout);
			//sum += word_bigrams[word].bigram_counts[i];
			//printf("  <%u,%u>=%u at pos %zu\n", prev_word, class_i, word_class_counts[prev_word * cmd_args.num_classes + class_i], ((size_t)prev_word * cmd_args.num_classes + class_i)); fflush(stdout);
		}
	}
	//printf("<w,c>: sum: %lu; [%u,%u,%u,%u,%u,%u,%u,%u,%u,%u...]\n", sum, word_class_counts[0], word_class_counts[1], word_class_counts[2], word_class_counts[3], word_class_counts[4], word_class_counts[5], word_class_counts[6], word_class_counts[7], word_class_counts[8], word_class_counts[9]);
}

double training_data_log_likelihood(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const count_arrays_t count_arrays, const word_count_t word_counts[const], const wclass_t word2class[const]) {
	const double backward_lambda = 1 - cmd_args.forward_lambda;

	// Transition Probs
	double transition_logprob = 0;
	// Bigrams
	#pragma omp parallel for num_threads(cmd_args.num_threads) reduction(+:transition_logprob)
	for (word_bigram_count_t ngram = 0; ngram < (powi(cmd_args.num_classes, 2)); ngram++) {
		const class_bigram_count_t bigram_count = count_arrays[1][ngram];
		if (!bigram_count) // bigram doesn't exist in training set
			continue;
		const wclass_t c_1 = ngram % cmd_args.num_classes;
		const wclass_t c_2 = ngram / cmd_args.num_classes;
		const wclass_count_t c_1_count = count_arrays[0][c_1];
		const wclass_count_t c_2_count = count_arrays[0][c_2];
		const double a = cmd_args.forward_lambda  * (bigram_count / (double)c_1_count);
		const double b = backward_lambda * (bigram_count / (double)c_2_count);
		transition_logprob += LOG2ADD(a,b) * bigram_count;
		//printf("ngram=%u, c_1=%u, #(c_1)=%lu, c_2=%u, #(c_2)=%lu, #(c_1,c_2)=%lu, trans_prob=%g\n", ngram, c_1, (unsigned long)c_1_count, c_2, (unsigned long)c_2_count, (unsigned long)bigram_count, transition_logprob); fflush(stdout);
	}

	// Emission Probs
	//long double emission_prob = 0;
	double emission_logprob = 0;
	//#pragma omp parallel for num_threads(cmd_args.num_threads) reduction(+:emission_logprob)
	for (word_id_t word = 0; word < model_metadata.type_count; word++) {
		//if (word == model_metadata.start_sent_id) // Don't tally emission prob for <s>
		//	continue;
		const word_count_t word_count = word_counts[word];
		if (!word_count) // Don't tally emission prob for <unk> if min-count is 1
			continue;
		const wclass_t class = word2class[word];
		const wclass_count_t class_count = count_arrays[0][class];
		emission_logprob += log2(word_count / (double)class_count) * word_count;
		//printf("word=%u, class=%u, emission_logprob=%g after += %g = log2(word_count=%lu / class_count=%u) * word_count=%lu\n", word, (unsigned int)class, emission_logprob, log2(word_count / (double)class_count) * word_count, (unsigned long)word_count, class_count, (unsigned long)word_count); fflush(stdout);
	}

	//printf("emission_logprob=%g, transition_logprob=%g, LL=%g\n", emission_logprob, transition_logprob, emission_logprob + transition_logprob);
	return emission_logprob + transition_logprob;
}

void init_count_arrays(const struct cmd_args cmd_args, count_arrays_t count_arrays) {
	for (unsigned char i = 1; i <= cmd_args.max_array; i++) { // Start with unigrams in count_arrays[0], ...
		count_arrays[i-1] = calloc(powi(cmd_args.num_classes, i), sizeof(wclass_count_t)); // powi() is in clustercat-math.c
		if (count_arrays[i-1] == NULL) {
			fprintf(stderr,  "%s: Error: Unable to allocate enough memory for %u-grams.  I tried to allocate %zu MB per thread (%zuB * %u^%u).  Reduce the number of desired classes using --classes (current value: %u)\n", argv_0_basename, i, sizeof(wclass_count_t) * powi(cmd_args.num_classes, i) / 1048576, sizeof(wclass_count_t), cmd_args.num_classes, i, cmd_args.num_classes ); fflush(stderr);
			exit(12);
		}
		//printf("Allocating %zu B (cmd_args.num_classes=%u^i=%u * sizeof(uint)=%zu)\n", (powi(cmd_args.num_classes, i) * sizeof(wclass_count_t)), cmd_args.num_classes, i, sizeof(wclass_count_t));
	}
}

void clear_count_arrays(const struct cmd_args cmd_args, count_arrays_t count_arrays) {
	for (unsigned char i = 1; i <= cmd_args.max_array; i++) { // Start with unigrams in count_arrays[0], ...
		memset(count_arrays[i-1], 0, powi(cmd_args.num_classes, i) * sizeof(wclass_count_t)); // powi() is in clustercat-math.c
	}
}

void free_count_arrays(const struct cmd_args cmd_args, count_arrays_t count_arrays) {
	for (unsigned char i = 1; i <= cmd_args.max_array; i++) { // Start with unigrams in count_arrays[0], ...
		free(count_arrays[i-1]);
	}
}
