#ifndef INCLUDE_CLUSTERCAT_TOKENIZE
#define INCLUDE_CLUSTERCAT_TOKENIZE

#include "clustercat.h"

sentlen_t tokenize_simple(char * restrict sent_string, char * restrict * restrict sent_words);
void tokenize_simple_free(char ** restrict sent_words, sentlen_t length);

#endif // INCLUDE_HEADER
