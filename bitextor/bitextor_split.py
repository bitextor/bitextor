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
import logging

from sentence_splitter import SentenceSplitter, SentenceSplitterException

from bitextor.utils.common import open_xz_or_gzip_or_plain
from bitextor.utils.common import ExternalTextProcessor


# True -> keep sentence
# False -> throw away
def filter_trash(sentence):
    digits_and_punctuation = string.punctuation + string.digits
    n = 0
    for c in sentence:
        if c == "\x00":
            return False
        if c in digits_and_punctuation:
            n = n + 1
    return n < len(sentence) // 2


def split_external(text, external_splitter, prune_type="words", prune_threshold=0, filter_bad_sentences=True):
    output, error_output, returncode = external_splitter.process(text)
    if returncode != 0:
        print(f"External sentence splitter existed with non-zero code: {returncode}", file=sys.stderr)
        print(error_output.strip(), file=sys.stderr)
        sys.exit(1)

    segments = output.strip().split("\n")
    # prune long sentences
    if prune_threshold and prune_type == "words":
        segments = [s for s in segments if not len(s.split()) > prune_threshold]
    elif prune_threshold and prune_type == "chars":
        segments = [s for s in segments if not len(s) > prune_threshold]

    if filter_bad_sentences:
        segments = [s for s in segments if filter_trash(s)]

    segmented_text = "\n".join(segments) + "\n"
    return segmented_text


def split_moses(text, moses_splitter, prune_type="words", prune_threshold=0, filter_bad_sentences=True):
    segments = moses_splitter.split(text)

    # prune long sentences
    if prune_threshold and prune_type == "words":
        segments = [s for s in segments if not len(s.split()) > prune_threshold]
    elif prune_threshold and prune_type == "chars":
        segments = [s for s in segments if not len(s) > prune_threshold]

    if filter_bad_sentences:
        segments = [s for s in segments if filter_trash(s)]

    segmented_text = "\n".join(segments) + "\n"
    return segmented_text


oparser = argparse.ArgumentParser(description="Tool that does sentence splitting on plain text")
oparser.add_argument('--text', default="-",
                     help="Plain text file")
oparser.add_argument('--sentence-splitter', dest='splitter', default=None,
                     help="Sentence splitter command line. If not provided, Moses split_sentences Python port "
                          "will be used")
oparser.add_argument('--langcode', default="en",
                     help="Language code for default sentence splitter and tokenizer")
oparser.add_argument('--customnbp',
                     help="Path for custom non breaking prefixes used by Moses Sentence Splitter Python port")
oparser.add_argument('--sentences-output', default="plain_sentences.xz", dest='sent_output',
                     help="Path of the output file that will contain sentence splitted text")
oparser.add_argument("--prune", dest="prune_threshold", type=int, default=0,
                     help="Prune sentences longer than n (words/characters)")
oparser.add_argument("--prune-type", choices={"words", "chars"}, default="words",
                     help="Prune sentences either by words or characters")
oparser.add_argument('--dont-filter', action='store_true',
                     help="By default, sentences which are detected to be very noisy or have very bad quality are discarded")
oparser.add_argument('--process-paragraphs', action='store_true',
                     help="Once the sentence had been base64-decoded, the second column contains the paragraph "
                          "identification which will be processed")

options = oparser.parse_args()

splitter = options.splitter
splitter_func = None
# no sentence splitter command provided, use moses:
if not splitter:
    splitter_func = split_moses
    try:
        if options.customnbp:
            splitter = SentenceSplitter(language=options.langcode, non_breaking_prefix_file=options.customnbp)
        else:
            splitter = SentenceSplitter(language=options.langcode)
    except SentenceSplitterException as e:
        sys.stderr.write(str(e) + "\n")
        splitter = SentenceSplitter(language='en')

# TODO check TODO in bitextor_tokenize.py about ExternalTextProcessor and ToolWrapper
# use custom sentence splitter via ExternalTextProcessor (inefficient):
else:
    splitter_func = split_external
    splitter = ExternalTextProcessor(os.path.expanduser(splitter))

with open_xz_or_gzip_or_plain(options.text) if options.text != "-" else sys.stdin as reader:
    for doc_idx, doc in enumerate(reader):
        sentences = ""
        content = ""

        try:
            content = base64.b64decode(doc.strip()).decode("utf-8").strip()
        except UnicodeDecodeError:
            logging.warning(f"unicode decoding error while processing doc #{doc_idx}")

        if options.process_paragraphs:
            content = content.split("\n")

            # Split each sentence of the paragraph and identify each of them with the corresponding paragraph
            for sent_idx, sentence in enumerate(content):
                paragraph = sentence.split("\t")

                if len(paragraph) == 1:
                    sentences += f"{paragraph[0]}\ts-1p-1"
                    logging.warning(f"could not get the paragraph identification data for the doc #{doc_idx}, sentence #{sent_idx}: using 's-1p-1'")
                    continue

                paragraph_text = paragraph[0]
                paragraph_id = paragraph[1]
                sentences_wo_paragraphs = splitter_func(paragraph_text, splitter, options.prune_type,
                                                        options.prune_threshold, not options.dont_filter).split("\n")

                # Add the paragraph data to the splitted sentences
                for idx in range(len(sentences_wo_paragraphs)):
                    if sentences_wo_paragraphs[idx].strip() != "":
                        sentences += f"{sentences_wo_paragraphs[idx]}\tp{paragraph_id}s{idx}\n"
        else:
            content = content.replace("\t", " ")
            sentences = splitter_func(content, splitter, options.prune_type, options.prune_threshold, not options.dont_filter)

        print(base64.b64encode(sentences.encode("utf-8")).decode("utf-8"))
