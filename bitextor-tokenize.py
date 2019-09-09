#!/usr/bin/env python3

import sys
import os
import argparse
import base64
import string
import ast
from external_processor import ExternalTextProcessor

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from utils.common import open_xz_or_gzip_or_plain


def get_lang_or_default(dict, lang):
    if not dict:
        return None
    if lang in dict:
        return dict[lang]
    elif "default" in dict:
        return dict["default"]
    else:
        return None

def extract_encoded_text(encoded, sent_tokeniser, word_tokeniser, morph_analyser):
    if not sent_tokeniser:
        print(encoded)
        return

    proc_sent = ExternalTextProcessor(sent_tokeniser.split())
    content = base64.b64decode(encoded).decode("utf-8").replace("\t", " ")
    tokenized_segs = proc_sent.process(content).strip()
    tokenized_filtered = ""

    for sent in tokenized_segs.split("\n"):
        if sum([1 for m in sent if m in string.punctuation + string.digits]) < len(sent) // 2:
            tokenized_filtered += sent + "\n"

    if not word_tokeniser:
        b64text = base64.b64encode(tokenized_filtered.lower().encode("utf-8"))
        print(b64text.decode())
        return

    proc_word = ExternalTextProcessor(word_tokeniser.split())
    tokenized_text = proc_word.process(tokenized_filtered)

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
oparser.add_argument('--langs', dest='langs', default=None,
                     help="List of  two-character language codes (comma-separated) to tokenize. "
                          "If not specified, every language will be processed")
oparser.add_argument('--sentence-splitters', dest='splitters', required=True,
                     help="A map of sentence splitter commands. "
                          "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                          "For languages that are not in this map but are in 'langs', the defaultScript will be used. "
                          "If defaultScript is not specified, language will be outputted in plain text.")
oparser.add_argument('--word-tokenizers', dest='tokenizers', required=True,
                     help="A map of word tokenisation commands. "
                          "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                          "For languages that are not in this map but are in 'langs', the defaultScript will be used. "
                          "If defaultScript is not specified, word tokenization for that language will be omitted.")
oparser.add_argument('--morph-analysers', dest='lemmatizers',
                     help="A map of morphological analysers. "
                          "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                          "For languages that are not in this map but a re in 'langs', the defaultScript will be used. "
                          "If defaultScript is not specified, morphological analysis for that language will be omitted.")

options = oparser.parse_args()

if options.langs:
    langs = options.langs.split(',')
else:
    langs = []

try:
    options.splitters = ast.literal_eval(options.splitters)
except:
    print("Sentence splitters incorrect format", file=sys.stderr)
    sys.exit(1)

try:
    options.tokenizers = ast.literal_eval(options.tokenizers)
except:
    print("Word tokenizers incorrect format", file=sys.stderr)
    sys.exit(1)

try:
    if options.lemmatizers:
        options.lemmatizers = ast.literal_eval(options.lemmatizers)
except:
    print("Morphological analysers incorrect format")
    sys.exit(1)

with open_xz_or_gzip_or_plain(options.text) as text_reader, \
        open_xz_or_gzip_or_plain(options.lang) as lang_reader:
    for line in text_reader:
        encodedtext = line.strip()
        lang = next(lang_reader, None).strip()

        if not langs or lang in langs:
            senttok = get_lang_or_default(options.splitters, lang)
            wordtok = get_lang_or_default(options.tokenizers, lang)
            morphtok = get_lang_or_default(options.lemmatizers, lang)
            extract_encoded_text(encodedtext, senttok, wordtok, morphtok)

