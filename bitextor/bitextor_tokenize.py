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

from mosestokenizer import MosesTokenizer

from bitextor.utils.common import open_xz_or_gzip_or_plain
from bitextor.utils.common import ExternalTextProcessor


# TODO ExternalTextProcessor has to open and close the external software every time
#  it is executed. Another option, whose performance is better, is ToolWrapper. The
#  problem with ToolWrapper is that the executed software cannot be doing buffering
#  since it is expected to obtain the ouput per each provided input, what might not happen
#  in the case of softwares which do buffering. One possible solution might be to
#  use ExternalTextProcessor as the default option and let the user configure if the
#  provided tool does not do buffering, and use ToolWrapper. It might be nice if the
#  taken decision would apply to every other scripts/steps (e.g. Bicleaner), since
#  this might avoid to hang up the pipeline due to mishandeling buffering for external
#  softwares


def tokenize_moses(text, word_tokeniser, morph_analyser):
    sentences = text.split('\n')
    tokenized_text = []
    for sentence in sentences:
        tokenized_text.append(" ".join(word_tokeniser(sentence)).strip())

    # don't do + "\n" because tokenized text ends with '' item
    tokenized_text = "\n".join(tokenized_text)

    if morph_analyser:
        tokenized_text = morph_analyser.process(tokenized_text)

    if tokenized_text == "":
        tokenized_text = "\n"

    return tokenized_text


def tokenize_external(text, word_tokeniser, morph_analyser):
    tokenized_text = word_tokeniser.process(text)

    if morph_analyser:
        tokenized_text = morph_analyser.process(tokenized_text)

    if tokenized_text == "":
        tokenized_text = "\n"

    return tokenized_text


oparser = argparse.ArgumentParser(description="Tool that tokenizes plain text in Base64")
oparser.add_argument('--text', default="-",
                     help="Plain text file Base64 encoded")
oparser.add_argument('--word-tokenizer', dest='tokenizer',
                     help="Word tokenisation command line. If not provided, Moses tokenizer.perl will be used")
oparser.add_argument('--morph-analyser', dest='lemmatizer',
                     help="Morphological analyser command line")
oparser.add_argument('--langcode', dest='langcode', default='en',
                     help="Language code for default sentence splitter and tokenizer")
oparser.add_argument('--dont-process-base64', action="store_true",
                     help="Process input and print output without Base64 encoding")

options = oparser.parse_args()

tokenizer = options.tokenizer
lemmatizer = options.lemmatizer
tokenizer_func = None

# no custom tokenizer is provided, use moses (internally uses tool wrapper)
if not tokenizer:
    tokenizer = MosesTokenizer(options.langcode)
    tokenizer_func = tokenize_moses
# use custom tokenizer via ExternalTextProcessor (inefficient)
else:
    tokenizer = ExternalTextProcessor(os.path.expanduser(tokenizer))
    tokenizer_func = tokenize_external

if lemmatizer:
    lemmatizer = ExternalTextProcessor(os.path.expanduser(lemmatizer))

with open_xz_or_gzip_or_plain(options.text) if options.text != "-" else sys.stdin as reader:
    for doc in reader:
        if options.dont_process_base64:
            content = doc
        else:
            content = base64.b64decode(doc.strip()).decode("utf-8")

        content = content.replace("\t", " ")
        tokenized = tokenizer_func(content, tokenizer, lemmatizer).lower()

        if options.dont_process_base64:
            print(tokenized, end="")
        else:
            print(base64.b64encode(tokenized.encode("utf-8")).decode("utf-8"))
