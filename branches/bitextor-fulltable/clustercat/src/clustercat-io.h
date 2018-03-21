#ifndef INCLUDE_CLUSTERCAT_IO
#define INCLUDE_CLUSTERCAT_IO

#include "clustercat.h"
#include "clustercat-data.h"

// Import
struct_model_metadata process_input(const struct cmd_args cmd_args, FILE *file, struct_map_word ** initial_word_map, struct_map_bigram ** initial_bigram_map, size_t *memusage);

#endif // INCLUDE_HEADER
