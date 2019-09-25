#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# 1. Read every line from .lett file
# 2. For each of them, clean the text and split it in words
# 3. Lowercase and create a bag of words
# 4. Creating a list with the words corresponding to every language and a list of the documents in which these
# words appear
#
# Output format:
# language      word    num_doc[:inc(num_doc)]*
#
# Generates .idx -> index
#

import os
import sys
import base64
import argparse

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from unicodepunct import get_unicode_punct
from utils.common import open_xz_or_gzip_or_plain


oparser = argparse.ArgumentParser(
    description="Script that reads the input of bitextor-ett2lett or bitextor-lett2lettr and uses the information "
                "about the files in a crawled website to produce an index with all the words in these files and the "
                "list of documents in which each of them appear")
oparser.add_argument('--text', dest='text',
                     help='File produced by bitextor-tokenize containing the tokenized text of all the records'
                     'in the WARC file encoded as base 64 (each line corresponds to a single record)', required=True)
oparser.add_argument('--lang', dest='lang',
                    help='File produced by bitextor-warc2preprocess containing the language of each of the records '
                    'in the WARC file encoded as base 64 (each line corresponds to a record)',required=True)
oparser.add_argument("-m", "--max-occ",
                     help="Maximum number of occurrences of a word in one language to be kept in the index", type=int,
                     dest="maxo", default=-1)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1",
                     required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2",
                     required=True)

options = oparser.parse_args()

docnumber = 0
word_map = {}

punctuation = get_unicode_punct()


with open_xz_or_gzip_or_plain(options.text) as text_reader:
    with open_xz_or_gzip_or_plain(options.lang) as lang_reader:

        for line in text_reader:
            ##################
            # Parsing the text:
            ##################
            tokenized_text = base64.b64decode(line.strip()).decode("utf-8")
            lang = next(lang_reader, None).strip()
            
            sorted_uniq_wordlist = set(tokenized_text.split())

            # Trimming non-aplphanumerics:
            clean_sorted_uniq_wordlist = [_f for _f in [w.strip(punctuation) for w in sorted_uniq_wordlist] if _f]
            sorted_uniq_wordlist = clean_sorted_uniq_wordlist

            for word in sorted_uniq_wordlist:
                if lang in word_map:
                    if word in word_map[lang]:
                        word_map[lang][word].append(docnumber)
                    else:
                        word_map[lang][word] = []
                        word_map[lang][word].append(docnumber)
                else:
                    word_map[lang] = {}
                    word_map[lang][word] = []
                    word_map[lang][word].append(docnumber)
            docnumber = docnumber + 1

for map_lang, map_vocabulary in list(word_map.items()):
    for map_word, map_doc in list(map_vocabulary.items()):
        if options.maxo == -1 or len(word_map[map_lang][map_word]) <= options.maxo:
            sorted_docs = sorted(word_map[map_lang][map_word], reverse=True)
            for doc_list_idx in range(0, len(sorted_docs) - 1):
                sorted_docs[doc_list_idx] = str(sorted_docs[doc_list_idx] - sorted_docs[doc_list_idx + 1])
            sorted_docs[len(sorted_docs) - 1] = str(sorted_docs[len(sorted_docs) - 1])
            print(map_lang + "\t" + map_word + "\t" + ":".join(reversed(sorted_docs)))
