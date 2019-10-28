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

import sys
import os
import argparse
import base64
import string
import ast
import lzma
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
        return encoded

    proc_sent = ExternalTextProcessor(sent_tokeniser.split())
    content = base64.b64decode(encoded).decode("utf-8").replace("\t", " ")
    tokenized_segs = proc_sent.process(content).strip()
    tokenized_filtered = ""

    for sent in tokenized_segs.split("\n"):
        if sum([1 for m in sent if m in string.punctuation + string.digits]) < len(sent) // 2:
            tokenized_filtered += sent + "\n"

    if not word_tokeniser:
        b64text = base64.b64encode(tokenized_filtered.lower().encode("utf-8"))
        return b64text.decode()

    proc_word = ExternalTextProcessor(word_tokeniser.split())
    tokenized_text = proc_word.process(tokenized_filtered)

    if morph_analyser:
        proc_morph = ExternalTextProcessor(morph_analyser.split())
        tokenized_text = proc_morph.process(tokenized_text)

    b64text = base64.b64encode(tokenized_text.lower().encode("utf-8"))
    return b64text.decode()


oparser = argparse.ArgumentParser(
    description="Tool that tokenizes (sentences, tokens and morphemes) plain text")
oparser.add_argument('--folder', dest='folder', help='Bitextorlang folder', required=True)
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

lang_files = {}

folder = os.fsencode(options.folder)
for langfolder in os.listdir(folder):
    lang = os.fsdecode(langfolder)
    if not os.path.isdir(options.folder+"/"+lang) or len(lang) > 2:
        continue
    fullname = os.path.join(options.folder, lang+"/plain_text.xz")
    if os.path.isfile(fullname) and (not langs or lang in langs):
        if lang not in lang_files:
            lang_files[lang] = lzma.open(os.path.join(options.folder, lang + "/plain_tokenized.xz"), "wb")
        with open_xz_or_gzip_or_plain(fullname) as text_reader:
            for line in text_reader:
                encodedtext = line.strip()
                senttok = get_lang_or_default(options.splitters, lang)
                wordtok = get_lang_or_default(options.tokenizers, lang)
                morphtok = get_lang_or_default(options.lemmatizers, lang)
                tokenized = extract_encoded_text(encodedtext, senttok, wordtok, morphtok)
                lang_files[lang].write("{}\n".format(tokenized).encode("utf-8"))
for lang in lang_files:
    lang_files[lang].close()
