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
from external_processor import ExternalTextProcessor

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from utils.common import open_xz_or_gzip_or_plain


def split_sentences(text, sent_tokeniser, prune_type="words", prune_threshold=0):
    proc_sent = ExternalTextProcessor(sent_tokeniser.split())
    segments = proc_sent.process(text).strip().split("\n")

    # prune long sentences
    if prune_threshold and prune_type == "words":
        segments = [s for s in segments if not len(s.split()) > prune_threshold]
    elif prune_threshold and prune_type == "chars":
        segments = [s for s in segments if not len(s) > prune_threshold]

    segments = [s for s in segments if sum([1 for char in s if char in (string.punctuation + string.digits)]) < len(s) // 2]
    
    if len(segments) != 0:
        segmented_text = "\n".join(segments) + "\n"
    else:
        segmented_text = ""
    return segmented_text


def tokenize(text, word_tokeniser, morph_analyser):
    proc_word = ExternalTextProcessor(word_tokeniser.split())
    tokenized_text = proc_word.process(text)

    if morph_analyser:
        proc_morph = ExternalTextProcessor(morph_analyser.split())
        tokenized_text = proc_morph.process(tokenized_text)

    return tokenized_text


oparser = argparse.ArgumentParser(description="Tool that tokenizes (sentences, tokens and morphemes) plain text")
oparser.add_argument('--text', dest='text', help='Plain text file', required=True)
oparser.add_argument('--sentence-splitter', dest='splitter', required=True, help="Sentence splitter commands")
oparser.add_argument('--word-tokenizer', dest='tokenizer', required=True, help="Word tokenisation command")
oparser.add_argument('--morph-analyser', dest='lemmatizer', default="", help="Morphological analyser command")
oparser.add_argument('--sentences-output', default="plain_sentences.xz", dest='sent_output',
                     help="Path of the output file that will contain sentence splitted text")
oparser.add_argument('--tokenized-output', default="plain_tokenized.xz", dest='tok_output',
                     help="Path of the output file that will contain sentence splitted and tokenized text")
oparser.add_argument("--prune", dest="prune_threshold", type=int, default=0,
                     help="Prune sentences longer than n (words/characters)", required=False)
oparser.add_argument("--prune-type", dest="prune_type", choices={"words", "chars"}, default="words",
                     help="Prune sentences either by words or characters", required=False)

options = oparser.parse_args()

with open_xz_or_gzip_or_plain(options.text) as reader, \
        open_xz_or_gzip_or_plain(options.sent_output, "w") as sent_writer, \
        open_xz_or_gzip_or_plain(options.tok_output, "w") as tok_writer:
    for doc in reader:
        content = base64.b64decode(doc.strip()).decode("utf-8").replace("\t", " ")
        sentences = split_sentences(content, os.path.expanduser(options.splitter), options.prune_type, options.prune_threshold)
        tokenized = tokenize(sentences, os.path.expanduser(options.tokenizer), os.path.expanduser(options.lemmatizer))
        sent_writer.write(base64.b64encode(sentences.encode("utf-8")) + b"\n")
        tok_writer.write(base64.b64encode(tokenized.lower().encode("utf-8")) + b"\n")
