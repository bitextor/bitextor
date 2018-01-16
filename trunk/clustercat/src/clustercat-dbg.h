#ifndef INCLUDE_CC_DBG_HEADER
#define INCLUDE_CC_DBG_HEADER

#include "clustercat.h"

void print_word_class_counts(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const word_class_count_t * restrict word_class_counts);

void print_word_bigrams(const struct_model_metadata model_metadata, const struct_word_bigram_entry * restrict word_bigrams, char ** restrict word_list);

#endif // INCLUDE_HEADER
