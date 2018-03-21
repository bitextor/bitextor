#include <stdio.h>
#include <errno.h>
#include "clustercat-import-class-file.h"
#include "clustercat-map.h"

// Parse TSV file input and overwrite relevant word mappings
void import_class_file(struct_map_word **word_map, wclass_t word2class[restrict], const char * restrict class_file_name, const wclass_t num_classes) {
	char * restrict line_end;
	char * restrict line = calloc(MAX_WORD_LEN + 9, 1);
	const word_id_t unk_id = map_find_id(word_map, UNKNOWN_WORD, -1);

	FILE *file = fopen(class_file_name, "r");
	if (!file) {
		fprintf(stderr, "%s: fopen of '%s' failed: %s.\n", argv_0_basename, class_file_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	while (fgets(line, MAX_WORD_LEN + 8, file) != 0) {

		line_end = strchr(line, '\n');
		*line_end = '\0';

		// Parse each line
		unsigned int keylen = strcspn(line, PRIMARY_SEP_STRING);
		line[keylen] = '\0'; // Split key and count
		char * restrict key = line;
		wclass_t class = atoi(line + keylen + 1);
		if (num_classes <= class) {
			fprintf(stderr,  " Error: Imported word classes from file \"%s\" must be in a range [0,%u-1].  Word \"%s\" has class %i.  If --num-classes is unset, a value is automatically chosen.  See --help\n", class_file_name, num_classes, key, class); fflush(stderr);
			exit(13);
		}
		//printf("keylen=%i, key=<<%s>>, class=<<%d>>\n", keylen, key, class);
		word_id_t key_int = map_find_id(word_map, key, unk_id);
		word2class[key_int] = class;
	}

	fclose(file);
	free(line);
}
