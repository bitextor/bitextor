#!/usr/bin/env python3

import sys
import os
import argparse
from external_processor import ExternalTextProcessor

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../utils")
from common import open_xz_or_gzip_or_plain


def extract_encoded_text(content,  word_tokeniser, morph_analyser):
    proc_word = ExternalTextProcessor(word_tokeniser.split())
    tokenized_text = proc_word.process(content)
    if morph_analyser:
        proc_morph = ExternalTextProcessor(morph_analyser.split())
        tokenized_text = proc_morph.process(tokenized_text)
    return tokenized_text.lower()


oparser = argparse.ArgumentParser()
oparser.add_argument('--text', dest='text', help='File containing text in lett format', required=True)
oparser.add_argument("--morph-analyser", help="Morphological analyser executable path", dest="morphtok", default=None)
oparser.add_argument("--word-tokeniser", help="Word tokeniser executable path", dest="wordtok", required=True)

options = oparser.parse_args()

with open_xz_or_gzip_or_plain(options.text) as text_reader:
    for line in text_reader:
        fields = line.split("\t")
        url = fields[0]
        text = fields[1]

        sent = extract_encoded_text(text.strip(), options.wordtok, options.morphtok).strip()
        print("{0}\t{1}".format(url, sent))
