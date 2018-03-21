#ifndef INCLUDE_CLUSTERCAT_HEADER
#define INCLUDE_CLUSTERCAT_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>			// log(), exp(), pow()
#include <libgen.h>			// basename()
#include <limits.h>			// USHRT_MAX, UINT_MAX
#include <errno.h>
#include "clustercat-math.h"	// powi()

// Defaults
#define PRIMARY_SEP_CHAR     '\t'
#define PRIMARY_SEP_STRING   "\t"
#define SECONDARY_SEP_CHAR   ' '
#define SECONDARY_SEP_STRING " "
#define TOK_CHARS            " \t\n"
#define UNKNOWN_WORD         "<unk>"
// Number of characters to read-in for each line
#define STDIN_SENT_MAX_CHARS 8000
#define MAX_WORD_LEN 128
#define MAX_WORD_PREDECESSORS 20000000
#define ENTROPY_TERMS_MAX 10000000

enum class_algos {EXCHANGE, BROWN, EXCHANGE_BROWN};
enum print_word_vectors {NO_VEC, TEXT_VEC, BINARY_VEC};

#include "clustercat-data.h" // bad. chicken-and-egg typedef deps

typedef unsigned short sentlen_t; // Number of words in a sentence
#define SENT_LEN_MAX USHRT_MAX
//typedef unsigned short wclass_t;  // Defined in clustercat-map.h
//typedef unsigned int   word_id_t; // Defined in clustercat-map.h
typedef word_count_t * * restrict count_arrays_t;
typedef word_count_t * restrict count_array_t;

typedef struct {
	unsigned long token_count;
	unsigned long line_count;
	word_id_t     type_count;
	word_id_t     start_sent_id; // need this for tallying emission probs
	word_id_t     end_sent_id; // need this for tallying emission probs
} struct_model_metadata;

// typedef {...} struct_word_bigram; // see clustercat-map.h

typedef struct { // This is for an array pointing to this struct having a pointer to an array of successors to a given word, as well as the length of that array
	word_id_t * predecessors;
	word_bigram_count_t * bigram_counts;
	unsigned long length;
	word_count_t headword_count;
} struct_word_bigram_entry;

char *argv_0_basename; // Allow for global access to filename

struct cmd_args {
	float           forward_lambda;
	wclass_t        num_classes;
	unsigned short  min_count : 12;
	signed char     verbose : 4;      // Negative values increasingly suppress normal output
	unsigned short  tune_cycles : 8;
	unsigned char   refine; // 0=no refinement; otherwise 2^n
	signed char     class_offset: 4;
	unsigned short  num_threads : 8;
	unsigned char   rev_alternate: 3; // How often to alternate using reverse pex.  0 == never, 1 == after every one normal pex cycles, ...
	unsigned char   max_array : 2;
	unsigned char   class_algo : 2;   // enum class_algos
	unsigned char   print_word_vectors : 2; // enum print_word_vectors
	bool ngram_input;
	bool print_freqs;
	bool unidirectional;
};

void populate_word_ids(struct_map_word **ngram_map, char * restrict unique_words[const], const word_id_t type_count);
void reassign_word_ids(struct_map_word **word_map, char * restrict word_list[restrict], word_id_t * restrict word_id_remap);
void build_word_count_array(struct_map_word **ngram_map, char * restrict unique_words[const], word_count_t word_counts[restrict], const word_id_t type_count);

void tally_class_ngram_counts(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const struct_word_bigram_entry word_bigrams[const], const wclass_t word2class[const], count_arrays_t count_arrays);
word_id_t filter_infrequent_words(const struct cmd_args cmd_args, struct_model_metadata * restrict model_metadata, struct_map_word ** ngram_map, word_id_t * restrict word_id_remap);
void init_clusters(const struct cmd_args cmd_args, word_id_t vocab_size, wclass_t word2class[restrict], const word_count_t word_counts[const], char * word_list[restrict]);
size_t set_bigram_counts(struct_word_bigram_entry * restrict word_bigrams, struct_map_bigram * bigram_map);
void build_word_class_counts(const struct cmd_args cmd_args, word_class_count_t * restrict word_class_counts, const wclass_t word2class[const], const struct_word_bigram_entry * const word_bigrams, const word_id_t type_count/*, char ** restrict word_list*/);
double training_data_log_likelihood(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const count_arrays_t count_arrays, const word_count_t word_counts[const], const wclass_t word2class[const]);

void init_count_arrays(const struct cmd_args cmd_args, count_arrays_t count_arrays);
void clear_count_arrays(const struct cmd_args cmd_args, count_arrays_t count_arrays);
void free_count_arrays(const struct cmd_args cmd_args, count_arrays_t count_arrays);

// Like atoi/strtol, but doesn't interpret each char's ascii value 0..9 .  Hence [104,101] ("he") -> 26725  (ie. (104*256)+101).  [3,7,11] -> 198411 (3*256*256) + (7*256) + 11)
// Using a class n-gram array is fast, at the expense of memory usage for lots of unattested ngrams, especially for higher-order n-grams.
// Trigrams are probably the highest order you'd want to use as an array, since the memory usage would be:  sizeof(wclass_t) * |C|^3   where |C| is the number of word classes.
// |C| can be represented using an unsigned short (16 bits == 65k classes) for exchange clustering, but probably should be an unsigned int (32 bit == 4 billion classes) for Brown clustering, since initially every word type is its own class.
inline size_t array_offset(wclass_t * pointer, const unsigned int max, const wclass_t num_classes) {
	register uint_fast8_t ptr_i = 1;
	register size_t total_offset = (*pointer);

	for (; ptr_i < max; ptr_i++) { // little endian
		//printf("1: atosize_t: pointer=%p; all vals: [%hu,%hu,%hu]; total_offset=%zu; max=%u\n", pointer, *pointer, *(pointer+1), *(pointer+2), total_offset, max); fflush(stdout);
		total_offset += (pointer[ptr_i]) * powi(num_classes, ptr_i);
		//printf("2: adding ((pointer[%u]=%u)* powi(%hu, %u)=%lu)=%lu\n", ptr_i, pointer[ptr_i], num_classes, ptr_i, powi(num_classes, ptr_i), pointer[ptr_i] * powi(num_classes, ptr_i)); fflush(stdout);
	}
	//printf("3: atosize_t: pointer=%p; val0=%hu; total_offset=%zu; max=%u\n\n", pointer, *pointer, total_offset, max); fflush(stdout);
	return total_offset;
}



#endif // INCLUDE_HEADER
