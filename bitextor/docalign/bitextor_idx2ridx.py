#!/usr/bin/env python3

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
# 1. Output from bitextor-lett2idx is read and an IDX file is obtained with an index of all the words in both
# languages and the list of documents where they occur
# 2. Words in both sides are translated by using the bilingual lexicon
# 3. Similarity metric based on bag-of-word-overlapping is computed
# 4. n-best documents are obtained for each document in the website
#
# Output format (RIDX):
# doc_id    [doc_id:score]+
#

import sys
import argparse
from collections import defaultdict
from operator import itemgetter
import re


def read_lett(f, docs):
    file = open(f, "r")
    fileid = 1
    for i in file:
        fields = i.strip().split("\t")
        if len(fields) >= 5:
            # To compute the edit distance at the level of characters, HTML tags must be encoded as characters and
            # not strings:
            docs[fileid] = fields[3]
        fileid += 1
    file.close()


#
# Building word indexes as dict files in python for both languages from the input IDX file
#
def fill_index(file, lang1, lang2, index1, index2):
    header = next(file).strip().split("\t")
    lang_idx = header.index("lang")
    word_idx = header.index("word")
    doc_idxs_idx = header.index("doc_idxs")

    for i in file:
        fields = i.strip().split("\t")

        if len(fields) == 3:
            lang = fields[lang_idx]
            word = fields[word_idx]
            doc_idxs = fields[doc_idxs_idx]

            if lang == lang1 or lang == lang2:
                documents = doc_idxs.split(":")
                acum = 1

                for j in documents:
                    acum += int(j)
                    if lang == lang1:
                        index1[acum].add(word)
                    else:
                        index2[acum].add(word)
    file.close()


#
# Loading bilingual lexicon (.dic)
#
def load_dictionaries(dictionary_path, lang1, lang2, dictionary):
    col_dic1 = -1
    col_dic2 = -1
    file = open(dictionary_path, "r")
    fields = file.readline().strip().split("\t")
    ind = 0
    for j in fields:
        if j == lang1:
            col_dic1 = ind
        elif j == lang2:
            col_dic2 = ind
        ind += 1
    for i in file:
        fields = i.strip().split("\t")
        if len(fields) == 2:
            dictionary[fields[col_dic2]].append(fields[col_dic1])
    file.close()


#
# Function that provides the set of translated words in a segments using a bilingual lexicon
#
def translate_words(index, dictionary, dictp, translatedindex):
    for i in index:
        translatedindex[i] = set([])
        counter = 0
        for word in index[i]:
            if word in dictionary:
                counter += 1
                translatedindex[i].update(dictionary[word])
        dictp[i] = counter


#
# The initial lexicon is extended by adding all those words that appear exactly the same in
# both sides (they are likely to be proper nouns, codes, dates, etc. that do not need to be translated).
#
def feed_dict_with_identical_words(index1, index2, dictionary):
    words_lang1 = set()
    for words in index1.values():
        words_lang1 = words_lang1.union(words)
    words_lang2 = set()
    for words in index2.values():
        words_lang2 = words_lang2.union(words)

    for w in words_lang1.intersection(words_lang2):
        dictionary[w].append(w)


oparser = argparse.ArgumentParser(
    description="Script that reads the output of bitextor-buildidx and builds an RIDX file (a list of documents and "
                "their corresponding n-best canidates to be parallel). To do so, a bag-of-word-overlapping metric is "
                "used to compare documents in both languages")
oparser.add_argument('idx', metavar='FILE', nargs='?',
                     help='File produced by bitextor-buildidx containing an index of the different words for every '
                          'language in the website and the list of documents in which they appear (if undefined, '
                          'the script will read from the standard input)',
                     default=None)
oparser.add_argument('-d',
                     help='Dictionary containing translations of words for the languages of the website; it is used '
                          'to compute the overlapping scores which allow to relate documents in both languages)',
                     dest="dictionary", required=True)
oparser.add_argument('-l',
                     help='LETT file; if it is provided, document pair candidates are provided only if they belong to '
                          'the same domain',
                     dest="lett", required=False, default=None)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1",
                     required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2",
                     required=True)
options = oparser.parse_args()

index_text1 = defaultdict(set)
index_text2 = defaultdict(set)
dic = defaultdict(list)
lista_words = []
found = {}
dict_words = {}
translated_index_text2 = {}
lett_documents = {}
ihost = None

# Loading bilingual lexicon
load_dictionaries(options.dictionary, options.lang1, options.lang2, dic)
if options.idx is None:
    reader = sys.stdin
else:
    reader = open(options.idx, "r")

# Loading IDX file
fill_index(reader, options.lang1, options.lang2, index_text1, index_text2)

# Extending the lexicon with words that are identical in both sides
feed_dict_with_identical_words(index_text1, index_text2, dic)

# Translating all the words in the segments in language 2 into language 1 using the bilingual lexicon
translate_words(index_text2, dic, dict_words, translated_index_text2)

if options.lett is not None:
    read_lett(options.lett, lett_documents)

for document_index1 in index_text1:
    if options.lett is not None:
        rx = re.match('(https?://)([^/]+)([^\?]*)(\?.*)?', lett_documents[document_index1])
        ihost = rx.group(2)

    similar = {}
    for document_index2 in index_text2:
        validpair = True
        if options.lett is not None:
            rx = re.match('(https?://)([^/]+)([^?]*)(\?.*)?', lett_documents[document_index2])
            jhost = rx.group(2)
            if jhost != ihost:
                validpair = False
        if validpair:
            c3 = index_text1[document_index1].intersection(translated_index_text2[document_index2])
            if len(c3) > 0 and int(dict_words[document_index2]) > 0:
                max_vocab = max(len(index_text1[document_index1]), len(index_text2[document_index2]))
                min_vocab = min(len(index_text1[document_index1]), len(index_text2[document_index2]))
                num_intersect_words = len(c3)
                num_trans_words_text2 = dict_words[document_index2]
                similar[document_index2] = (float(min_vocab) / float(max_vocab)) * (
                    float(num_intersect_words) / float(num_trans_words_text2))

    if len(similar) > 0:
        similar = sorted(list(similar.items()), key=itemgetter(1), reverse=True)
    found[document_index1] = []
    for document_index2 in similar:
        found[document_index1].append(str(document_index2[0]) + ":" + str(document_index2[1]))

# Print output header
print("doc_idx\tbest_matches_idx_and_score")

# For each document, we obtain the 10-best candidates with highest score.
for document_index in found:
    if len(found[document_index]) > 10:
        num_best_candidates = 10
    else:
        num_best_candidates = len(found[document_index])

    candidatestring = str(document_index) + "\t"

    for document_index2 in range(num_best_candidates):
        if document_index2 == 0:
            candidatestring += str(found[document_index][document_index2])
        else:
            candidatestring += "_" + str(found[document_index][document_index2])

    print(candidatestring)
