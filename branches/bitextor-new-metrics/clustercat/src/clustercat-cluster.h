#ifndef INCLUDE_CC_CLUSTER_HEADER
#define INCLUDE_CC_CLUSTER_HEADER

#include "clustercat.h"

typedef struct { // This is for an array pointing to this struct having a pointer to an array of word_id's all within the same class. We also keep track of the length of that array.
	word_id_t * words;
	unsigned int length;
} struct_class_listing;

void cluster(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const word_count_t word_counts[const], char * word_list[restrict], wclass_t word2class[], const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts);

void print_words_and_vectors(FILE * out_file, const struct cmd_args cmd_args, const struct_model_metadata model_metadata, char * word_list[restrict], wclass_t word2class[], const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts);

void post_exchange_brown_cluster(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, wclass_t word2class[], const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts, count_arrays_t count_arrays);

void build_entropy_terms(const struct cmd_args cmd_args, float * restrict entropy_terms, const unsigned int entropy_terms_max);

void get_class_listing(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const wclass_t word2class[const], struct_class_listing * restrict class2words);
void free_class_listing(const struct cmd_args cmd_args, struct_class_listing * restrict class2words);
#endif // INCLUDE_HEADER
