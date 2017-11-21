#include "clustercat-dbg.h"

void print_word_class_counts(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const word_class_count_t * restrict word_class_counts) {
	for (wclass_t class = 0; class < cmd_args.num_classes; class++) {
		printf("Class=%u   Offsets=%u,%u,...%u:\n\t", class, class, class+cmd_args.num_classes, (model_metadata.type_count-1) * cmd_args.num_classes + class);
		for (word_id_t word = 0; word < model_metadata.type_count; word++) {
			printf("#(<%u,%hu>)=%u  ", word, class, word_class_counts[word * cmd_args.num_classes + class]);
		}
		printf("\n");
	}
	fflush(stdout);
}

void print_word_bigrams(const struct_model_metadata model_metadata, const struct_word_bigram_entry * restrict word_bigrams, char ** restrict word_list) {
	printf("word_bigrams:\n"); fflush(stdout);
	for (word_id_t word_i = 0; word_i < model_metadata.type_count; word_i++) {
		printf("  %18s=%u -> {%lu, [", word_list[word_i], word_i, word_bigrams[word_i].length); fflush(stdout);
		for (word_id_t word_j = 0; word_j < word_bigrams[word_i].length; word_j++) {
			if (word_j > 0)
				printf(", ");
			printf("%s=%u (%ux)", word_list[word_bigrams[word_i].predecessors[word_j]], word_bigrams[word_i].predecessors[word_j], word_bigrams[word_i].bigram_counts[word_j]);
		}
		printf("]}\n"); fflush(stdout);
	}
}
