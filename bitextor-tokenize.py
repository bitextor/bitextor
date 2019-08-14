#!/usr/bin/env python3

import sys
import os
import argparse
import base64
import lzma
from external_processor import ExternalTextProcessor

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from utils.common import open_xz_or_gzip_or_plain


def extract_encoded_text(encoded, sent_tokeniser, word_tokeniser, morph_analyser):
    proc_sent = ExternalTextProcessor(sent_tokeniser.split())
    proc_word = ExternalTextProcessor(word_tokeniser.split())
    content = base64.b64decode(encoded).decode("utf-8").replace("\t", " ")
    tokenized_segs = proc_sent.process(content).strip()
    tokenized_text = proc_word.process(tokenized_segs)
    if morph_analyser:
        proc_morph = ExternalTextProcessor(morph_analyser.split())
        tokenized_text = proc_morph.process(tokenized_text)
    b64text = base64.b64encode(tokenized_text.lower().encode("utf-8"))
    print(b64text.decode())


oparser = argparse.ArgumentParser(
    description="Tool that tokenizes (sentences, tokens and morphemes) plain text")
oparser.add_argument('--text', dest='text',
                     help='File produced by bitextor-warc2preprocess containing the text of all the records in the '
                          'WARC file encoded as base 64 (each line corresponds to a record)',
                     required=True)
oparser.add_argument('--lang', dest='lang',
                     help='File produced by bitextor-warc2preprocess containing the language of each of the records '
                          'in the WARC file encoded as base 64 (each line corresponds to a record)',
                     required=True)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1",
                     required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2",
                     required=True)
oparser.add_argument("--morph-analyser_sl", help="Path to the morphological analyser for SL to TL",
                     dest="morphtok1", default=None)
oparser.add_argument("--morph-analyser_tl", help="Path to the morphological analyser for TL to SL",
                     dest="morphtok2", default=None)
oparser.add_argument("--sent-tokeniser_sl", help="Path to the sentence tokeniser for SL", dest="senttok1",
                     required=True)
oparser.add_argument("--sent-tokeniser_tl", help="Path to the sentence tokeniser for TL", dest="senttok2",
                     required=True)
oparser.add_argument("--word-tokeniser_sl", help="Path to the word tokeniser for SL", dest="wordtok1", required=True)
oparser.add_argument("--word-tokeniser_tl", help="Path to the word tokeniser for TL", dest="wordtok2", required=True)

options = oparser.parse_args()

with open_xz_or_gzip_or_plain(options.text) as text_reader, \
        open_xz_or_gzip_or_plain(options.lang) as lang_reader:
    for line in text_reader:
        encodedtext = line.strip()
        lang = next(lang_reader, None).strip()

        if lang == options.lang1:
            extract_encoded_text(encodedtext, options.senttok1, options.wordtok1, options.morphtok1)
        elif lang == options.lang2:
            extract_encoded_text(encodedtext, options.senttok2, options.wordtok2, options.morphtok2)
        else:
            print(encodedtext)

