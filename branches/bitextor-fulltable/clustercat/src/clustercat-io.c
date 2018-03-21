#include <stdio.h>
#include <string.h>
#include "clustercat.h"
#include "clustercat-data.h"
#include "clustercat-array.h"
#include "clustercat-io.h"

struct_model_metadata process_input(const struct cmd_args cmd_args, FILE *file, struct_map_word ** initial_word_map, struct_map_bigram ** initial_bigram_map, size_t *memusage) {
	struct_model_metadata model_metadata = {0};
	map_update_count(initial_word_map, UNKNOWN_WORD, 0, 0); // initialize entry for <unk>, <s>, and </s>
	map_update_count(initial_word_map, "<s>", 0, 1);
	map_update_count(initial_word_map, "</s>", 0, 2);
	const word_id_t unk_id   = map_find_id(initial_word_map, UNKNOWN_WORD, 0);
	const word_id_t start_id = map_find_id(initial_word_map, "<s>", 1);
	const word_id_t end_id   = map_find_id(initial_word_map, "</s>", 2);
	const size_t sizeof_struct_map_word   = sizeof(struct_map_word);
	const size_t sizeof_struct_map_bigram = sizeof(struct_map_bigram);
	model_metadata.type_count = 3; // start with <unk>, <s>, and </s>, and <unk>.

	// n-gram input
	if (cmd_args.ngram_input) {
		char line[STDIN_SENT_MAX_CHARS];
		register unsigned int strlen_line = 0;
		register unsigned long line_num = 1;
		register char * count_split_pos = NULL;
		register char * word_split_pos  = NULL;
		register unsigned long count = 0;

		while (!feof(file)) {
			if (! fgets(line, STDIN_SENT_MAX_CHARS, file))
				break;
			if (*line == '\n') // ignore empty lines
				continue;
			strlen_line = strlen(line);
			if (strlen_line == STDIN_SENT_MAX_CHARS-1)
				fprintf(stderr, "\n%s: Warning: Input line too long, at buffer line %lu. The full line was:\n%s\n", argv_0_basename, line_num, line);
			line[strlen_line-1] = '\0'; // rm newline

			// Split words from counts
			count_split_pos = strchr(line, '\t');
			*count_split_pos = '\0';
			if (count_split_pos == NULL) {
				fprintf(stderr, "\n%s: Warning: Malformed n-gram input line number %lu. The line was:\n%s\n", argv_0_basename, line_num, line); fflush(stderr);
			} else {
				count = strtoul(count_split_pos+1, NULL, 10);
			}

			// Try to split word1 from word2
			word_split_pos  = strchr(line, ' ');

			if (word_split_pos) { // Line has bigrams
				*word_split_pos = '\0';

				// Lookup each word
				const word_id_t w1 = map_find_id(initial_word_map, line, unk_id);
				const word_id_t w2 = map_find_id(initial_word_map, word_split_pos+1, unk_id);
				if (w1 == unk_id || w2 == unk_id) // Unseen word(s) in bigram
					fprintf(stderr, "%s: Warning: Unseen word(s) in bigram '%s %s' on input line %lu will be assigned to '%s'. Otherwise, include in unigram counts first.\n", argv_0_basename, line, word_split_pos+1, line_num, UNKNOWN_WORD);

				// Form bigram
				const struct_word_bigram bigram = {w1, w2};

				// Update bigram count
				if (map_update_bigram(initial_bigram_map, &bigram, count)) // increment previous+</s> bigram in bigram map
					*memusage += sizeof_struct_map_bigram;

			} else { // Line has unigrams
				if (model_metadata.type_count == map_update_count(initial_word_map, line, count, model_metadata.type_count)) { // <unk>'s word_id is set to 0.
					model_metadata.type_count++;
					*memusage += sizeof_struct_map_word;
				}

			}

			//if (word_split_pos) // line could be unigram count
			//	printf("w1=<<%s>>; w2=<<%s>>; count=<<%s>>==%lu\n", line, word_split_pos+1, count_split_pos+1, count);
			//else
			//	printf("w1=<<%s>>; count=<<%s>>==%lu\n", line, count_split_pos+1, count);
			//fflush(stdout);

			line_num++;
		}


	// Normal text input
	} else {
		char curr_word[MAX_WORD_LEN + 1]; curr_word[MAX_WORD_LEN] = '\0';
		register unsigned int chars_in_sent = 0;
		register int ch = 0;
		unsigned int curr_word_pos = 0;
		unsigned int prev_word_id = start_id;

		while (!feof(file)) {
			ch = getc(file);
			chars_in_sent++;
			//printf("«%c» ", ch); fflush(stdout);
			if (ch == ' ' || ch == '\t' || ch == '\n') { // end of a word

				if (chars_in_sent > STDIN_SENT_MAX_CHARS) { // Line too long
					curr_word_pos = 0;
					curr_word[0] = '\0'; // truncate word
				} else {
					curr_word[curr_word_pos] = '\0'; // terminate word
				}

				//printf("chars_in_sent=%u; max_chars=%u; curr_word=%s\n", chars_in_sent, STDIN_SENT_MAX_CHARS, curr_word); fflush(stdout);

				if (!strncmp(curr_word, "", 1)) { // ignore empty words, due to leading, trailing, and multiple spaces
					//printf("skipping empty word; ch=«%c»\n", ch); fflush(stdout);
					if (ch == '\n') { // trailing spaces require more stuff to do
						const struct_word_bigram bigram = {prev_word_id, end_id};
						if (map_increment_bigram(initial_bigram_map, &bigram)) // increment previous+</s> bigram in bigram map
							*memusage += sizeof_struct_map_bigram;
						chars_in_sent = 0;
						prev_word_id = start_id;
						model_metadata.line_count++;
					}
					continue;
				}
				//printf("curr_word=%s, prev_id=%u\n", curr_word, prev_word_id); fflush(stdout);
				model_metadata.token_count++;
				curr_word_pos = 0;
				// increment current word in word map
				const word_id_t curr_word_id = map_increment_count(initial_word_map, curr_word, model_metadata.type_count); // <unk>'s word_id is set to 0.

				if (curr_word_id == model_metadata.type_count) { // previous call to map_increment_count() had a new word
					model_metadata.type_count++;
					*memusage += sizeof_struct_map_word;
				}

				// increment previous+current bigram in bigram map
				const struct_word_bigram bigram = {prev_word_id, curr_word_id};
				//printf("{%u,%u}\n", prev_word_id, curr_word_id); fflush(stdout);
				if (map_increment_bigram(initial_bigram_map, &bigram)) // true if bigram is new
					*memusage += sizeof_struct_map_bigram;

				//printf("process_input(): curr_word=<<%s>>; curr_word_id=%u, prev_word_id=%u\n", curr_word, curr_word_id, prev_word_id); fflush(stdout);
				if (ch == '\n') { // end of line
					const struct_word_bigram bigram = {curr_word_id, end_id};
					if (map_increment_bigram(initial_bigram_map, &bigram)) // increment previous+</s> bigram in bigram map
						*memusage += sizeof_struct_map_bigram;
					chars_in_sent = 0;
					prev_word_id = start_id;
					model_metadata.line_count++;
				} else {
					prev_word_id = curr_word_id;
				}

			} else { // normal character;  within a word
				if (curr_word_pos > MAX_WORD_LEN) // word is too long; do nothing until space or newline
					continue;
				else
					curr_word[curr_word_pos++] = ch;
			}
		}
	}

	// Set counts of <s> and </s> once, based on line_count
	map_update_count(initial_word_map, "<s>", model_metadata.line_count, 1);
	map_update_count(initial_word_map, "</s>", model_metadata.line_count, 2);
	return model_metadata;
}
