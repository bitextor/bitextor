#include "clustercat-map.h"

inline bool map_increment_bigram(struct_map_bigram **map, const struct_word_bigram * bigram) {
	struct_map_bigram *local_s;
	HASH_FIND(hh, *map, bigram, sizeof(struct_word_bigram), local_s); // id already in the hash?
	if (local_s == NULL) {
		local_s = (struct_map_bigram *)malloc(sizeof(struct_map_bigram));
		//memcpy(local_s->key, bigram, sizeof(struct_word_bigram));
		local_s->key = *bigram;
		local_s->count = 1;
		HASH_ADD(hh, *map, key, sizeof(struct_word_bigram), local_s);
		return true;
	} else {
		(local_s->count)++;
		return false;
	}
}

inline bool map_update_bigram(struct_map_bigram **map, const struct_word_bigram * bigram, const word_bigram_count_t count) {
	struct_map_bigram *local_s;
	HASH_FIND(hh, *map, bigram, sizeof(struct_word_bigram), local_s); // id already in the hash?
	if (local_s == NULL) {
		local_s = (struct_map_bigram *)malloc(sizeof(struct_map_bigram));
		//memcpy(local_s->key, bigram, sizeof(struct_word_bigram));
		local_s->key = *bigram;
		local_s->count = count;
		HASH_ADD(hh, *map, key, sizeof(struct_word_bigram), local_s);
		return true;
	} else {
		local_s->count += count;
		return false;
	}
}

void map_print_bigrams(struct_map_bigram **bigram_map, char **word_list) {
	struct_map_bigram *entry, *tmp;
	struct_word_bigram bigram_key;
	word_id_t w_1, w_2;
	word_bigram_count_t count;

	printf("bigram_map:\n");
	HASH_ITER(hh, *bigram_map, entry, tmp) {
		count          = entry->count;
		bigram_key     = entry->key;
		w_1            = bigram_key.word_1;
		w_2            = bigram_key.word_2;
		if (w_1 == (word_id_t)-1 || w_2 == (word_id_t)-1) // Don't print dummy values
			continue;
		printf(" {%s=%u, %s=%u}: #=%u\n", word_list[w_1], w_1, word_list[w_2], w_2, count);
		//printf(" {%u, %u}: #=%u\n", w_1, w_2, count); fflush(stdout);
	}
	printf("\n"); fflush(stdout);
}

void remap_and_rev_bigram_map(struct_map_bigram ** initial_bigram_map, struct_map_bigram ** new_bigram_map, struct_map_bigram ** new_bigram_map_rev, word_id_t * restrict word_id_remap, const word_id_t real_unk_id) {
	// Iterates through initial bigram hash map and builds a new hash map based on the mapping of old word id's to new ids.  Alongside this, it also builds a reversed counterpart.
	struct_map_bigram *entry, *tmp;
	struct_word_bigram orig_bigram, new_bigram, new_bigram_rev;
	word_id_t w_1, w_2;
	word_bigram_count_t count;
	//printf("initial_bigram_map hash_count=%u\n", HASH_COUNT(initial_bigram_map));
	//printf("word_id_remap71: [%u,%u,%u,%u,%u,%u,...]\n", word_id_remap[0], word_id_remap[1], word_id_remap[2], word_id_remap[3], word_id_remap[4], word_id_remap[5]);

	HASH_ITER(hh, *initial_bigram_map, entry, tmp) {
		count          = entry->count;
		orig_bigram    = entry->key;
		w_1            = word_id_remap[orig_bigram.word_1];
		w_2            = word_id_remap[orig_bigram.word_2];
		if (w_1 == (word_id_t) -1) // reassign temporary placeholder unk_id to final unk_id
			w_1 = real_unk_id;
		if (w_2 == (word_id_t) -1)
			w_2 = real_unk_id;
		new_bigram     = (struct_word_bigram) {w_1, w_2};
		new_bigram_rev = (struct_word_bigram) {w_2, w_1};
		//printf("remap_and_rev_bigram_map: count=%u, orig_w_1=%u, new_w_1=%u, orig_w_2=%u, new_w_2=%u\n", count, orig_bigram.word_1, w_1, orig_bigram.word_2, w_2); fflush(stdout);

		//#pragma omp parallel sections // Both bigram listing and reverse bigram listing can be done in parallel
		{
			//#pragma omp section
			{ map_update_bigram(new_bigram_map, &new_bigram, count); }
			//const word_bigram_count_t bigram_count = map_update_bigram(&new_bigram_map, &new_bigram, count);
			//printf("map_update_bigram: {%u,%u} += %u; now %u\n", new_bigram.word_1, new_bigram.word_2, count, bigram_count);
			//#pragma omp section
			{ map_update_bigram(new_bigram_map_rev, &new_bigram_rev, count); }
		}
	}
}

inline void map_add_entry(struct_map_word **map, char * restrict entry_key, const word_count_t count) { // Based on uthash's docs
	struct_map_word *local_s;

	//HASH_FIND_STR(*map, entry_key, local_s); // id already in the hash?
	//if (local_s == NULL) {
		local_s = (struct_map_word *)malloc(sizeof(struct_map_word));
		unsigned short strlen_entry_key = strlen(entry_key);
		local_s->key = malloc(strlen_entry_key + 1);
		strcpy(local_s->key, entry_key);
		HASH_ADD_KEYPTR(hh, *map, local_s->key, strlen_entry_key, local_s);
	//}
	local_s->count = count;
}

inline void map_add_class(struct_map_word_class **map, const char * restrict entry_key, const unsigned long word_count, const wclass_t entry_class) {
	struct_map_word_class *local_s;

	//HASH_FIND_STR(*map, entry_key, local_s); // id already in the hash?
	//if (local_s == NULL) {
		local_s = (struct_map_word_class *)malloc(sizeof(struct_map_word_class));
		strncpy(local_s->key, entry_key, KEYLEN-1);
		HASH_ADD_STR(*map, key, local_s);
	//}
	local_s->word_count = word_count;
	local_s->class = entry_class;
}

inline void map_update_class(struct_map_word_class **map, const char * restrict entry_key, const unsigned short entry_class) {
	struct_map_word_class *local_s;

	HASH_FIND_STR(*map, entry_key, local_s); // id already in the hash?
	if (local_s == NULL) {
		local_s = (struct_map_word_class *)malloc(sizeof(struct_map_word_class));
		strncpy(local_s->key, entry_key, KEYLEN-1);
		HASH_ADD_STR(*map, key, local_s);
	}
	local_s->class = entry_class;
}

inline void map_set_word_id(struct_map_word **map, const char * restrict entry_key, const word_id_t word_id) {
	struct_map_word *local_s; // local_s->word_id uninitialized here; assign value after filtering

	#pragma omp critical (map_set_word_id_lookup)
	{
		HASH_FIND_STR(*map, entry_key, local_s); // id already in the hash?
	}
	if (local_s == NULL) {
		printf("Error: map_set_word_id(): word '%s' should already be in word_map\n", entry_key); // Shouldn't happen
		exit(5);
	}
	#pragma omp critical (map_set_word_id_assignment)
	{ local_s->word_id = word_id; }
}

inline word_id_t map_increment_count(struct_map_word **map, const char * restrict entry_key, const word_id_t word_id) { // Based on uthash's docs
	struct_map_word *local_s; // local_s->word_id uninitialized here; assign value after filtering

	#pragma omp critical (map_increment_count_lookup)
	{
		HASH_FIND_STR(*map, entry_key, local_s); // id already in the hash?
		if (local_s == NULL) {
			local_s = (struct_map_word *)malloc(sizeof(struct_map_word));
			local_s->count = 0;
			local_s->word_id = word_id;
			unsigned short strlen_entry_key = strlen(entry_key);
			local_s->key = malloc(strlen_entry_key + 1);
			strcpy(local_s->key, entry_key);
			HASH_ADD_KEYPTR(hh, *map, local_s->key, strlen_entry_key, local_s);
		}
	}
	#pragma omp critical (map_increment_count_increment)
	{ ++local_s->count; }
	//printf("map: count of %s is now %u\n", entry_key, local_s->count);
	return local_s->word_id;
}

inline wclass_count_t map_increment_count_fixed_width(struct_map_class **map, const wclass_t entry_key[const]) { // Based on uthash's docs
	struct_map_class *local_s;
	size_t sizeof_key = sizeof(wclass_t) * CLASSLEN;
	//printf("map++: sizeof_key=%zu, CLASSLEN=%u, cls_entry=[%hu,%hu,%hu,%hu]\n", sizeof_key, CLASSLEN, entry_key[0], entry_key[1], entry_key[2], entry_key[3]);

	//#pragma omp critical // not needed since each thread gets its own class_map
	{
		//printf("***41***: sizeof_key=%zu, sizeof(wclass_t)=%zu, CLASSLEN=%u, key=<%u,%u,%u,%u>\n", sizeof_key, sizeof(wclass_t), CLASSLEN, entry_key[0], entry_key[1], entry_key[2], entry_key[3]); fflush(stdout);
		HASH_FIND(hh, *map, entry_key, sizeof_key, local_s); // id already in the hash?
		if (local_s == NULL) {
			local_s = (struct_map_class *)malloc(sizeof(struct_map_class));
			local_s->count = 0;
			memcpy(local_s->key, entry_key, sizeof_key);
			HASH_ADD(hh, *map, key, sizeof_key, local_s);
		}
		//printf("\t***42***: count: %u\n", local_s->count); fflush(stdout);
	}
	#pragma omp critical (map_increment_count_fixed_width_increment)
	{ ++local_s->count; }
	//printf("map: count of [%hu,%hu,%hu,%hu] is now %u\n", entry_key[0],entry_key[1],entry_key[2],entry_key[3], local_s->count);
	return local_s->count;
}

inline wclass_count_t map_find_count_fixed_width(struct_map_class *map[const], const wclass_t entry_key[const]) { // Based on uthash's docs
	struct_map_class *local_s;
	size_t sizeof_key = sizeof(wclass_t) * CLASSLEN;
	wclass_count_t local_count = 0;

	HASH_FIND(hh, *map, entry_key, sizeof_key, local_s); // id already in the hash?
	if (local_s != NULL) { // Deal with OOV
		local_count = local_s->count;
	}
	//printf("map: count=%u for cls_entry=[%hu,%hu,%hu,%hu]\n", local_count, entry_key[0], entry_key[1], entry_key[2], entry_key[3]);
	return local_count;
}

inline word_id_t map_update_count(struct_map_word **map, const char * restrict entry_key, const word_count_t count, const word_id_t word_id) { // Based on uthash's docs
	struct_map_word *local_s;

	#pragma omp critical
	{
		HASH_FIND_STR(*map, entry_key, local_s); // id already in the hash?
		if (local_s == NULL) {
			local_s = (struct_map_word *)malloc(sizeof(struct_map_word));
			local_s->count = count;
			local_s->word_id = word_id;
			unsigned short strlen_entry_key = strlen(entry_key);
			local_s->key = malloc(strlen_entry_key + 1);
			strcpy(local_s->key, entry_key);
			HASH_ADD_KEYPTR(hh, *map, local_s->key, strlen_entry_key, local_s);
		} else {
			local_s->count += count;
		}
	}
	return local_s->word_id;
}

inline word_count_t map_find_count(struct_map_word *map[const], const char * restrict entry_key) { // Based on uthash's docs
	struct_map_word *local_s;
	word_count_t local_count = 0;

	HASH_FIND_STR(*map, entry_key, local_s);	// local_s: output pointer
	if (local_s != NULL) { // Deal with OOV
		local_count = local_s->count;
	}
	return local_count;
}

inline word_id_t map_find_id(struct_map_word *map[const], const char * restrict entry_key, const word_id_t unknown_id) { // Based on uthash's docs
	struct_map_word *local_s;
	word_id_t local_id = unknown_id;

	HASH_FIND_STR(*map, entry_key, local_s);
	if (local_s != NULL) { // Deal with OOV
		local_id = local_s->word_id;
	}
	return local_id;
}

struct_map_word map_find_entry(struct_map_word *map[const], const char * restrict entry_key) { // Based on uthash's docs
	struct_map_word *local_s;

	HASH_FIND_STR(*map, entry_key, local_s);
	return *local_s;
}

inline wclass_t get_class(struct_map_word_class *map[const], const char * restrict entry_key, const wclass_t unk) {
	struct_map_word_class *local_s;

	HASH_FIND_STR(*map, entry_key, local_s);	// local_s: output pointer
	if (local_s != NULL) { // Word is found
		return local_s->class;
	} else { // Word is not found
		return unk;
	}
}

word_id_t get_keys(struct_map_word *map[const], char *keys[]) {
	struct_map_word *entry, *tmp;
	word_id_t number_of_keys = 0;

	HASH_ITER(hh, *map, entry, tmp) {
		// Build-up array of keys
		unsigned short wlen = strlen(entry->key);
		keys[number_of_keys] = (char *) malloc(wlen + 1);
		strcpy(keys[number_of_keys], entry->key);
		//printf("key=%s, i=%lu, count=%u\n", entry->key, (unsigned long)number_of_keys, entry->count);
		number_of_keys++;
	}
	return number_of_keys;
}

word_id_t get_ids(struct_map_word *map[const], word_id_t word_ids[restrict]) { // most useful if map is already sorted by count; then you can directly map from old id to new id.
	struct_map_word *entry, *tmp;
	word_id_t number_of_keys = 0; // 0-2 are reserved for <unk>, <s>, and </s>

	HASH_ITER(hh, *map, entry, tmp) {
		//word_ids[number_of_keys] = entry->word_id; // Build-up array of word_id's, from new id to old one
		const word_id_t word_id = entry->word_id;
		//if (word_id < 3) // don't change id's for <unk>, <s>, or </s>
		//	continue;
		word_ids[word_id] = number_of_keys; // Build-up array of word_id's, from old id to new one
		//printf("get_ids: old_id=%u\n", word_id); fflush(stdout);
		number_of_keys++;
	}
	return number_of_keys;
}

void delete_entry(struct_map_word **map, struct_map_word *entry) { // Based on uthash's docs
	HASH_DEL(*map, entry);	// entry: pointer to deletee
	free(entry->key); // key is a malloc'd string
	free(entry);
}

void delete_all(struct_map_word **map) {
	struct_map_word *current_entry, *tmp;

	HASH_ITER(hh, *map, current_entry, tmp) { // Based on uthash's docs
		HASH_DEL(*map, current_entry);	// delete it (map advances to next)
		free(current_entry);	// free it
	}
}

void delete_all_class(struct_map_class **map) {
	struct_map_class *current_entry, *tmp;

	HASH_ITER(hh, *map, current_entry, tmp) { // Based on uthash's docs
		HASH_DEL(*map, current_entry);	// delete it (map advances to next)
		free(current_entry);	// free it
	}
}

void delete_all_bigram(struct_map_bigram **map) {
	struct_map_bigram *current_entry, *tmp;

	HASH_ITER(hh, *map, current_entry, tmp) { // Based on uthash's docs
		HASH_DEL(*map, current_entry);	// delete it (map advances to next)
		free(current_entry);	// free it
	}
}

void print_words_and_classes(FILE * out_file, word_id_t type_count, char **word_list, const word_count_t word_counts[const], const wclass_t word2class[const], const int class_offset, const bool print_freqs) {
	struct_map_word_class *map = NULL;

	for (word_id_t word_id = 0; word_id < type_count; word_id++) { // Populate new word2class_map, so we can do fun stuff like primary- and secondary-sort easily
		//printf("adding %s=%hu to temp word2class_map\n", word_list[word_id], word2class[word_id]); fflush(stdout);
		map_add_class(&map, word_list[word_id], (unsigned long)word_counts[word_id], word2class[word_id]);
	}

	sort_by_key(&map); // Tertiary sort, alphabetically by key
	word_class_sort_by_count(&map); // Secondary sort, by count
	sort_by_class(&map); // Primary sort, numerically by class

	struct_map_word_class *s;
	for (s = map; s != NULL; s = (struct_map_word_class *)(s->hh.next)) {
		fprintf(out_file, "%s\t%li", s->key, (long)(s->class) + class_offset);
		if (print_freqs)
			fprintf(out_file, "\t%lu", (long unsigned)(s->word_count));
		fprintf(out_file, "\n");
		HASH_DEL(map, s);	// delete it (map advances to next)
		free(s->key);	// free it
		//fprintf(stderr, "49.11: next=%zu\n", (struct_map_word_class *)(s->hh.next)); fflush(stderr);
	}
}

int count_sort(struct_map_word *a, struct_map_word *b) { // Based on uthash's docs
	return (b->count - a->count); // sort descending: most frequent to least frequent
}

void sort_by_count(struct_map_word **map) { // Based on uthash's docs
	HASH_SORT(*map, count_sort);
}

int id_sort(struct_map_word *a, struct_map_word *b) {
	return (a->word_id - b->word_id); // sort ascending
}

void sort_by_id(struct_map_word **map) {
	HASH_SORT(*map, id_sort);
}

int word_class_count_sort(struct_map_word_class *a, struct_map_word_class *b) {
	return (b->word_count - a->word_count); // sort descending: most frequent to least frequent
}

void word_class_sort_by_count(struct_map_word_class **map) {
	HASH_SORT(*map, word_class_count_sort);
}

int key_sort(struct_map_word_class *a, struct_map_word_class *b) {
	return strcmp(a->key, b->key);
}

void sort_by_key(struct_map_word_class **map) {
	HASH_SORT(*map, key_sort);
}

int class_sort(struct_map_word_class *a, struct_map_word_class *b) { // Based on uthash's docs
	return (a->class - b->class);
}

void sort_by_class(struct_map_word_class **map) {
	HASH_SORT(*map, class_sort);
}

inline int bigram_sort_word_1(struct_map_bigram *a, struct_map_bigram *b) { // Based on uthash's docs
	return ((a->key).word_1 - (b->key).word_1);
}

inline int bigram_sort_word_2(struct_map_bigram *a, struct_map_bigram *b) { // Based on uthash's docs
	return ((a->key).word_2 - (b->key).word_2);
}

void sort_bigrams(struct_map_bigram **map) {
	HASH_SORT(*map, bigram_sort_word_2);
	//HASH_SORT(*map, bigram_sort_word_1);
}

unsigned long map_count(struct_map_word *map[const]) {
	return HASH_COUNT(*map);
}

unsigned long map_print_entries(struct_map_word **map, const char * restrict prefix, const char sep_char, const word_count_t min_count) {
	struct_map_word *entry, *tmp;
	unsigned long number_of_entries = 0;

	HASH_ITER(hh, *map, entry, tmp) {
		if (entry->count >= min_count) {
			printf("%s%s%c%lu\n", prefix, entry->key, sep_char, (unsigned long)entry->count);
			number_of_entries++;
		}
	}
	return number_of_entries;
}
