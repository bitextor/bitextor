#include <string.h>
#include "clustercat-tokenize.h"

// Simple threadsafe tokenization for plaintext, copying words into **sent_words
// Remember to free using tokenize_simple_free()
sentlen_t tokenize_simple(char * restrict sent_string, char * restrict * restrict sent_words) {
	sentlen_t i;
	char * restrict pch;

	sent_words[0] = "<s>";

	for (i = 1, pch = sent_string; i < SENT_LEN_MAX ; i++) {
		sentlen_t toklen = strcspn(pch, " \n\t");

		if (toklen == 0) { // End of sentence
			sent_words[i] = "</s>";
			break;
		}

		sent_words[i] = malloc(toklen+1);
		strncpy(sent_words[i], pch, toklen); // Threadsafe copy doesn't touch original
		sent_words[i][toklen] = '\0';

		pch += toklen+1;
	}

	return i;
}

void tokenize_simple_free(char ** restrict sent_words, sentlen_t length) {
	sentlen_t i = 1;
	for (; i < length-1; ++i) { // Assumes word_0 is <s> and word_sentlen is </s>, which weren't malloc'd
	    free(sent_words[i]);
	}
	free(sent_words);
}
