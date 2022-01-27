#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#  This file is part of Bitextor.
#
#  Bitextor is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bitextor is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bitextor.  If not, see <https://www.gnu.org/licenses/>.

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

import base64
import argparse

from bitextor.utils.unicodepunct import get_unicode_punct
from bitextor.utils.common import open_xz_or_gzip_or_plain


oparser = argparse.ArgumentParser(
    description="Script that reads the tokenised documents and produces an index with all the words "
                "in these files and the list of documents in which each of them appear")
oparser.add_argument('--text1', dest='text1',
                     help='File produced by bitextor-tokenize containing the tokenized text of all the records'
                     'in the WARC file encoded as base 64 (each line corresponds to a single record) for SL Language', required=True)
oparser.add_argument('--text2', dest='text2',
                     help='File produced by bitextor-tokenize containing the tokenized text of all the records'
                     'in the WARC file encoded as base 64 (each line corresponds to a single record) for TL Language', required=True)
oparser.add_argument("-m", "--max-occ",
                     help="Maximum number of occurrences of a word in one language to be kept in the index", type=int,
                     dest="maxo", default=-1)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1",
                     required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2",
                     required=True)

options = oparser.parse_args()

word_map = {}
punctuation = get_unicode_punct()
doc_idx = 0

for file_path, lang in [(options.text1, options.lang1), (options.text2, options.lang2)]:
    if lang not in word_map:
        word_map[lang] = {}

    with open_xz_or_gzip_or_plain(file_path) as text_reader:
        # Process documents
        for line in text_reader:
            # Decode the text (current document)
            tokenized_text = base64.b64decode(line.strip()).decode("utf-8")

            # Get unique words from the current document
            sorted_uniq_wordlist = sorted(set(tokenized_text.split()))

            # Trimming non-aplphanumerics
            sorted_uniq_wordlist = [_f for _f in [w.strip(punctuation) for w in sorted_uniq_wordlist] if _f]

            # Process every unique word from the current document
            for word in sorted_uniq_wordlist:
                if word not in word_map[lang]:
                    word_map[lang][word] = []

                word_map[lang][word].append(doc_idx)

        doc_idx += 1

# Print output header
print("lang\tword\tdoc_idxs")

for map_lang, map_vocabulary in list(word_map.items()):
    for map_word, map_doc in list(map_vocabulary.items()):
        if options.maxo == -1 or len(word_map[map_lang][map_word]) <= options.maxo: # If there are many occurrences, the word might be a stop word
            sorted_docs = sorted(word_map[map_lang][map_word], reverse=True)

            # Encode doc idxs
            # Example: [500, 300, 299, 5] -> [500 - 300, 300 - 299, 299 - 5, 5] = [200, 1, 294, 5]
            # TODO why is this encoding being used? Is it really needed?
            for doc_list_idx in range(0, len(sorted_docs) - 1):
                sorted_docs[doc_list_idx] = str(sorted_docs[doc_list_idx] - sorted_docs[doc_list_idx + 1])

            sorted_docs[-1] = str(sorted_docs[-1])

            # lang->en word->hello docs->[5, 294, 1, 200]
            print(map_lang + "\t" + map_word + "\t" + ":".join(reversed(sorted_docs)))
        # else: -> detected as stop word and we want to avoid them
