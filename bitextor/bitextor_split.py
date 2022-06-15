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

    segments = '\n'.join([segment.strip() for segment in segments if segment.strip() != ''])

    if return_list:
        return segments

    segmented_text = "\n".join(segments) + "\n"

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
                     help="Once the sentence had been base64-decoded, the second column contains the paragraph "
                          "identification which will be processed")

options = oparser.parse_args()

splitter = options.splitter
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

        try:
            content = base64.b64decode(doc.strip()).decode("utf-8")
        except UnicodeDecodeError:
            logging.warning("unicode decoding error while processing doc #%d", doc_idx)

        if options.process_paragraphs:
            content = content.rstrip().split("\n")

            # Split each sentence of the paragraph and identify each of them with the corresponding paragraph
            for sent_idx, sentence in enumerate(content, 1):
                paragraph = sentence.split("\t")

                if len(paragraph) == 1:
                    sentences += f"{paragraph[0]}\tp-1s-1\n"
                    logging.warning("could not get the paragraph identification data for the doc #%d, sentence #%d: using 'p-1s-1'",
                                    doc_idx, sent_idx)
                    continue

                paragraph_text = paragraph[0]
                paragraph_id = int(paragraph[1]) + 1 # Start at 1
                sentences_wo_paragraphs = split_segments(paragraph_text, splitter_func, options.prune_type,
                                                         options.prune_threshold, not options.dont_filter, return_list=True)

                # Add the paragraph data to the splitted sentences
                for idx in range(len(sentences_wo_paragraphs)):
                    sentences += f"{sentences_wo_paragraphs[idx]}\t" \
                                 f"p{paragraph_id}/{len(content)}s{idx + 1}/{len(sentences_wo_paragraphs)}\n"
        else:
            content = content.strip().replace("\t", " ")
            content = '\n'.join([c.strip() for c in content.split('\n')])
            sentences = split_segments(content, splitter_func, options.prune_type, options.prune_threshold, not options.dont_filter)

        sentences = base64.b64encode(sentences.encode("utf-8")).decode("utf-8")

        writer.write(f"{sentences}\n")
