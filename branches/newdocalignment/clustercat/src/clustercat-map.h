#ifndef INCLUDE_CLUSTERCAT_MAP_HEADER
#define INCLUDE_CLUSTERCAT_MAP_HEADER

#include <stdio.h>
#include <stdbool.h>
#include "uthash.h"

#ifdef ATA_STORE_KHASH
 #include "khash.h"
 KHASH_MAP_INIT_STR(struct_khash_float, float);
#endif

// Defaults
#define KEYLEN 80
#define CLASSLEN 3 // Longest possible class ngram to store
typedef unsigned short wclass_t;            // Max number of word classes
typedef unsigned int   wclass_count_t;      // Max count of a given word class
typedef unsigned int   word_id_t;           // Max number of words
typedef unsigned int   word_count_t;        // Max count of a given word class
typedef unsigned int   word_bigram_count_t; // Max count of a given word bigram
typedef unsigned int   class_bigram_count_t; // Max count of a given class bigram
typedef unsigned int   word_class_count_t;  // Max count of a given <word, class> tuple

typedef struct {
	word_id_t word_1;
	word_id_t word_2;
} struct_word_bigram;


typedef struct { // We need an O(1) map that we can iterate over later
	struct_word_bigram key;
	word_bigram_count_t count;
	UT_hash_handle hh;	// makes this structure hashable
} struct_map_bigram;

typedef struct {
	char * restrict key;
	word_count_t count;
	word_id_t word_id;
	UT_hash_handle hh;	// makes this structure hashable
} struct_map_word;

typedef struct { // Maps a class to its count
	wclass_t key[CLASSLEN];
	wclass_count_t count;
	UT_hash_handle hh;	// makes this structure hashable
} struct_map_class;

typedef struct { // Maps a word to its class
	char key[KEYLEN];
	unsigned long word_count;
	wclass_t class;
	UT_hash_handle hh;	// makes this structure hashable
} struct_map_word_class;

void map_add_entry(struct_map_word **map, char * restrict entry_key, const word_count_t count);

void map_add_class(struct_map_word_class **map, const char * restrict entry_key, const unsigned long word_count, const wclass_t entry_class);

void map_update_class(struct_map_word_class **map, const char * restrict entry_key, const wclass_t entry_class);

void map_set_word_id(struct_map_word **map, const char * restrict entry_key, const word_id_t word_id);

word_id_t map_increment_count(struct_map_word **map, const char * restrict entry_key, const word_id_t word_id);

wclass_count_t map_increment_count_fixed_width(struct_map_class **map, const wclass_t entry_key[const]);

bool map_increment_bigram(struct_map_bigram **map, const struct_word_bigram * bigram);
bool map_update_bigram(struct_map_bigram **map, const struct_word_bigram * bigram, const word_bigram_count_t count);
void map_print_bigrams(struct_map_bigram **map, char **word_list);
void remap_and_rev_bigram_map(struct_map_bigram ** initial_bigram_map, struct_map_bigram ** new_bigram_map, struct_map_bigram ** new_bigram_map_rev, word_id_t * restrict word_id_remap, const word_id_t real_unk_id);

word_id_t map_update_count(struct_map_word **map, const char * restrict entry_key, const word_count_t count, const word_id_t word_id);

struct_map_word map_find_entry(struct_map_word *map[const], const char * restrict entry_key);
word_count_t map_find_count(struct_map_word *map[const], const char * restrict entry_key);
wclass_count_t map_find_count_fixed_width(struct_map_class *map[const], const wclass_t entry_key[const]);

word_id_t map_find_id(struct_map_word *map[const], const char * restrict entry_key, const word_id_t unknown_id);

wclass_t get_class(struct_map_word_class *map[const], const char * restrict entry_key, const wclass_t unk);

word_id_t get_keys(struct_map_word *map[const], char *keys[]);
word_id_t get_ids(struct_map_word *map[const], word_id_t word_ids[restrict]);

void sort_by_class(struct_map_word_class **map);
void sort_by_key(struct_map_word_class **map);
void sort_by_id(struct_map_word **map);
void sort_by_count(struct_map_word **map);
void word_class_sort_by_count(struct_map_word_class **map);
void sort_bigrams(struct_map_bigram **map);

unsigned long map_count(struct_map_word *map[const]);

unsigned long map_print_entries(struct_map_word **map, const char * restrict prefix, const char sep_char, const word_count_t min_count);
void print_words_and_classes(FILE * out_file, word_id_t type_count, char **word_list, const word_count_t word_counts[const], const wclass_t word2class[const], const int class_offset, const bool print_freqs);

void delete_all(struct_map_word **map);
void delete_all_class(struct_map_class **map);
void delete_all_bigram(struct_map_bigram **map);
void delete_entry(struct_map_word **map, struct_map_word *entry);

#endif // INCLUDE_HEADER
