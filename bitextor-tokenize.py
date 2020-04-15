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

from mosestokenizer import MosesTokenizer

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from utils.common import open_xz_or_gzip_or_plain
from utils.common import ExternalTextProcessor


def tokenize_moses(text, word_tokeniser, morph_analyser):
    sentences = text.split('\n')
    tokenized_text = []
    for sentence in sentences:
        tokenized_text += word_tokeniser(sentence) + ["\n"]
    tokenized_text = " ".join(tokenized_text).replace('\n ', '\n').strip()

    if morph_analyser:
        tokenized_text = proc_morph.process(tokenized_text)

    return tokenized_text

def tokenize_external(text, word_tokeniser, morph_analyser):
    tokenized_text = word_tokeniser.process(text)
    
    if morph_analyser:
        tokenized_text = morph_analyser.process(tokenized_text)
    
    return tokenized_text

oparser = argparse.ArgumentParser(description="Tool that tokenizes (sentences, tokens and morphemes) plain text")
oparser.add_argument('--text', dest='text', help='Plain text file', required=True)
oparser.add_argument('--word-tokenizer', dest='tokenizer', default=None, help="Word tokenisation command line. If not provided, Moses tokenizer.perl will be used")
oparser.add_argument('--morph-analyser', dest='lemmatizer', default="", help="Morphological analyser command line")
oparser.add_argument('--langcode', dest='langcode', default="en", help="Language code for default sentence splitter and tokenizer")

options = oparser.parse_args()

tokenizer = options.tokenizer

# no custom tokenizer is provided, use moses (internally uses tool wrapper)
if not tokenizer:
    tokenizer = MosesTokenizer(options.langcode)
    tokenizer_func = tokenize_moses
# use custom tokenizer via ExternalTextProcessor (inefficient)
else:
    tokenizer = ExternalTextProcessor(os.path.expanduser(tokenizer).split())
    tokenizer_func = tokenize_external

lemmatizer = options.lemmatizer
if lemmatizer:
    lemmatizer = ExternalTextProcessor(os.path.expanduser(lemmatizer).split())

with open_xz_or_gzip_or_plain(options.text) as reader, \
    for doc in reader:
        content = base64.b64decode(doc.strip()).decode("utf-8").replace("\t", " ")
        tokenized = tokenize(content, tokenizer, lemmatizer).lower()
        print(base64.b64encode(tokenized.encode("utf-8")))
