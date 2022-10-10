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
from loomchild.segmenter import LoomchildSegmenter

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


def split_segments(text, splitter_unary_func, prune_type="words", prune_threshold=0, filter_bad_sentences=True, return_list=False):
    segments = splitter_unary_func(text)

    # prune long sentences
    if prune_threshold and prune_type == "words":
        segments = [s for s in segments if not len(s.split()) > prune_threshold]
    elif prune_threshold and prune_type == "chars":
        segments = [s for s in segments if not len(s) > prune_threshold]

    if filter_bad_sentences:
        segments = [s for s in segments if filter_trash(s)]

    segments = [s.strip() for s in segments if s.strip() != '']

    if return_list:
        return segments

    segmented_text = '\n'.join(segments) + '\n'

    return segmented_text


oparser = argparse.ArgumentParser(description="Tool that does sentence splitting on plain text")
oparser.add_argument("--text", default="-",
                     help="Plain text file")
oparser.add_argument("--sentence-splitter", dest="splitter",
                     help="Sentence splitter command line. If not provided, loomchild loomchild-segment Python port "
                          "will be used")
oparser.add_argument("--langcode", default="en",
                     help="Language code for default sentence splitter and tokenizer")
oparser.add_argument("--customnbp",
                     help="Path for custom non breaking prefixes used by Moses Sentence Splitter Python port")
oparser.add_argument("--sentences-output", dest="sent_output", default="-",
                     help="Path of the output file that will contain sentence splitted text")
oparser.add_argument("--prune", dest="prune_threshold", type=int, default=0,
                     help="Prune sentences longer than n (words/characters)")
oparser.add_argument("--prune-type", choices={"words", "chars"}, default="words",
                     help="Prune sentences either by words or characters")
oparser.add_argument("--dont-filter", action="store_true",
                     help="By default, sentences which are detected to be very noisy or have very bad quality are discarded")
oparser.add_argument("--process-paragraphs", action="store_true",
                     help="Paragraph identification, starting in 0, is expected to be in the second column of the input file")
oparser.add_argument("--propagate-metadata", action="store_true",
                     help="All columns, starting from 2nd, will be propagated. If --process-paragraphs is set, columns starting"
                          " from 3rd column will be propagated")

options = oparser.parse_args()

splitter = options.splitter
process_paragraphs = options.process_paragraphs
propagate_metadata = options.propagate_metadata

splitter_func = lambda s: s.split('\n')

# Get splitter
if not splitter or splitter == "loomchild":
    # Loomchild is the default sentence splitter
    splitter_func = LoomchildSegmenter(options.langcode).get_document_segmentation
elif splitter == "moses":
    try:
        if options.customnbp:
            splitter_func = SentenceSplitter(language=options.langcode, non_breaking_prefix_file=options.customnbp)
        else:
            splitter_func = SentenceSplitter(language=options.langcode)
    except SentenceSplitterException as e:
        sys.stderr.write(str(e) + "\n")
        splitter_func = SentenceSplitter(language='en')

    splitter_func = splitter_func.split
elif splitter == "none":
    pass

# TODO check TODO in bitextor_tokenize.py about ExternalTextProcessor and ToolWrapper
# use custom sentence splitter via ExternalTextProcessor (inefficient):
else:
    splitter_func = ExternalTextProcessor(os.path.expanduser(splitter)).process

with open_xz_or_gzip_or_plain(options.text) if options.text != "-" else sys.stdin as reader, \
     open(options.sent_output, 'w') if options.sent_output != "-" else sys.stdout as writer:
    for doc_idx, doc in enumerate(reader, 1):
        sentences = ""
        content = ""
        doc = doc.strip()
        skip = False

        try:
            content = base64.b64decode(doc).decode("utf-8").strip(' \n')
        except UnicodeDecodeError:
            logging.error("Unicode decoding error: skipping document #%d", doc_idx)

            skip = True

        if skip:
            # TODO should we try to get the content of the file? We shouldn't write the content directly
            #  since the previous error for which we skip the sentence splitting can't handle correctly
            #  the metadata, what might lead to unexpected results in further stages. Furthermore, the
            #  malformed BASE64 content might lead to further stages to fail as well
            continue

        content = content.split("\n")

        # Split each sentence of the paragraph and identify each of them with the corresponding paragraph
        for sent_idx, sentence in enumerate(content, 1):
            column = sentence.split('\t')
            paragraph_text = column[0].strip()

            if process_paragraphs and len(column) == 1:
                sentences += f"{paragraph_text}\tp-1s-1\n"

                logging.error("Could not get the paragraph identification data for the doc #%d, sentence #%d: using 'p-1s-1'", doc_idx, sent_idx)

                continue

            sentences_wo_paragraphs = split_segments(paragraph_text, splitter_func, options.prune_type,
                                                        options.prune_threshold, not options.dont_filter, return_list=True)
            suffix_offset = 2 if process_paragraphs else 1
            suffix = ('\t' if len(column) > suffix_offset else '') + '\t'.join(column[suffix_offset:]) + '\n' if propagate_metadata else '\n'

            if process_paragraphs:
                try:
                    paragraph_id = column[1] # Starts at 1 and includes total number of paragraphs
                except ValueError as e:
                    raise Exception(f"Couldn't process document #{doc_idx}, sentence #{sent_idx}") from e

            for idx in range(len(sentences_wo_paragraphs)):
                infix = f"\tp{paragraph_id}s{idx + 1}/{len(sentences_wo_paragraphs)}" if process_paragraphs else '' # Paragraph data
                sentences += f"{sentences_wo_paragraphs[idx]}{infix}{suffix}"

        sentences = base64.b64encode(sentences.encode("utf-8")).decode("utf-8")

        writer.write(f"{sentences}\n")
