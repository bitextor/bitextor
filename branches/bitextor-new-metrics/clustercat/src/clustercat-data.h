#ifndef INCLUDE_CLUSTERCAT_DATA_HEADER
#define INCLUDE_CLUSTERCAT_DATA_HEADER

#include "clustercat-map.h"
//#include "clustercat-tree.h"

// Thanks Dipstick
#define STR(x) #x
#define SHOW_DEFINE(x) printf("%s=%s\n", #x, STR(x))
//	SHOW_DEFINE(DATA_STRUCT_FLOAT_NAME); // for example

// Default to storing word-word entries in hash table using uthash
// You can change this by compiling with -DATA_STORE_TREE_LCRS or -DATA_STORE_TRIE
#if defined ATA_STORE_KHASH // https://github.com/attractivechaos/klib
 #define DATA_STRUCT_FLOAT_HUMAN_NAME "khash_map"
 #define DATA_STRUCT_FLOAT_NAME word_word_float_khash
 #define DATA_STRUCT_FLOAT_ADDR 
 #define DATA_STRUCT_FLOAT_TYPE kh_struct_khash_float_t
 #define DATA_STRUCT_FLOAT_TYPE_IN_STRUCT kh_struct_khash_float_t
 #define DATA_STRUCT_FLOAT_SIZE sizeof(kh_struct_khash_float_t)
 #define DECLARE_DATA_STRUCT_FLOAT KHASH_MAP_INIT_STR(DATA_STRUCT_FLOAT_TYPE, float);
 #define INIT_DATA_STRUCT_FLOAT khash_t(struct_khash_float) * DATA_STRUCT_FLOAT_NAME = kh_init(struct_khash_float);
 #define UPDATE_ENTRY_FLOAT(db,key,val) { \
	 int ret; \
	 khint_t k = kh_put(struct_khash_float, (&db), (key), &ret); \
	 if (!ret) kh_del(struct_khash_float, (&db), (k)); \
	 kh_value((&db), (k)) = (val); \
 }
 #define FIND_ENTRY_FLOAT(db,key) ( kh_get(struct_khash_float, (db), (key)))
 //#define PRINT_ENTRIES_FLOAT(db, prefix, sep_char, min_count) ({ \
 //    unsigned long number_of_entries = 0; \
 //    for (khint_t k = kh_begin(db); k != kh_end(db); ++k) \
 //   	if (kh_exist(db, k)) { \
 //   		printf("foobar\n"); \
 ////		printf("%s%s%c%i\n", prefix, entry->key, sep_char, entry->count);
 //   		number_of_entries++; \
 //   	} \
 //    return number_of_entries; \
 //})
 #define PRINT_ENTRIES_FLOAT(db, prefix, sep_char, min_count) (1)
#endif

typedef struct {
	struct_map_word *word_map;
	struct_map_word *word_word_map;
	struct_map_word *ngram_map;
	struct_map_word *class_map;
	char **unique_words;
} struct_model_maps;


#endif // INCLUDE_HEADER
