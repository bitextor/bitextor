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

import os
import argparse
import base64
import string
import logging
import re

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
oparser.add_argument("--metadata-output", default="-",
                     help="Metadata output file")
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
oparser.add_argument("--expected-metadata-fields", type=int, default=-1,
                     help="Number of expected metadata fields. It will be used only if --propagate-metadata is set")

options = oparser.parse_args()

splitter = options.splitter
process_paragraphs = options.process_paragraphs
propagate_metadata = options.propagate_metadata
expected_metadata_fields = options.expected_metadata_fields if propagate_metadata else -1

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
        logging.error("%s", str(e))
        splitter_func = SentenceSplitter(language='en')

    splitter_func = splitter_func.split
elif splitter == "none":
    pass

# TODO check TODO in bitextor_tokenize.py about ExternalTextProcessor and ToolWrapper
# use custom sentence splitter via ExternalTextProcessor (inefficient):
else:
    splitter_func = ExternalTextProcessor(os.path.expanduser(splitter)).process

same_output_file = options.sent_output == options.metadata_output # Not perfect approach for detecting the same path
not_only_text = process_paragraphs or propagate_metadata
no_columns = set()
output_decode = options.sent_output == '-'
metadata_decode = options.metadata_output == '-'

if same_output_file and options.sent_output != '-':
    logging.warning("Same output and metadata output files: it will be handled using different columns")

with open_xz_or_gzip_or_plain(options.text) as reader, \
     open_xz_or_gzip_or_plain(options.sent_output, "wb") as writer, \
     open_xz_or_gzip_or_plain(options.metadata_output, "wb") as metadata_writer:
    for doc_idx, doc in enumerate(reader, 1):
        sentences = ""
        metadata = ""
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
            content = ""

            # Add the necessary missing data (fake)
            content += '\t-1:-1' if process_paragraphs else '' # Add paragraph identification
            content += '\t' * expected_metadata_fields if expected_metadata_fields > 0 else '' # Add metadata

        content = content.split("\n")
        content_not_processed = True
        fake_document = False

        # Split each sentence of the paragraph and identify each of them with the corresponding paragraph
        for sent_idx, sentence in enumerate(content, 1):
            columns = sentence.split('\t')
            paragraph_text = columns[0].strip()
            current_metadata = ''
            empty_but_valid_metadata = False

            # Some sanity checks
            if len(columns) == 1 and not_only_text:
                logging.error("Expected columns is >1, but got 1: document %d, sentence %d: "
                              "ignoring all columns but the first", doc_idx, sent_idx)
            if len(columns) > 1 and not not_only_text:
                logging.error("Expected columns is 1, but got %d: document %d, sentence %d: "
                              "ignoring all columns but the first", len(columns), doc_idx, sent_idx)
            if len(columns) > 2 and process_paragraphs and not propagate_metadata:
                logging.error("Expected columns is 2, but got %d: document %d, sentence %d: "
                              "ignoring all columns but the first and second", len(columns), doc_idx, sent_idx)

            no_columns.add(len(columns))

            if len(columns) == 1 and process_paragraphs:
                logging.error("Could not get the paragraph identification data: document %d, sentence %d: "
                              "using 'p-1:-1sM/N' (it might not even appear)", doc_idx, sent_idx)

                columns.append("-1:-1") # Fake paragraph identification

            # Sentence splitting
            sentences_wo_paragraphs = split_segments(paragraph_text, splitter_func, options.prune_type,
                                                     options.prune_threshold, not options.dont_filter, return_list=True)
            paragraph_id = 0
            total_paragraphs = 0
            if process_paragraphs:
                pattern = re.compile("^(-?[0-9]+):(-?[0-9]+)$")
                m = re.match(pattern, columns[1])
                if m:
                    paragraph_id = int(m.group(1)) # Starts at 1
                    total_paragraphs = int(m.group(2))

                    if paragraph_id > total_paragraphs:
                        logging.warning(f"Paragraph id > total paragraphs (bug?): {paragraph_id} > {total_paragraphs}")

                else:
                    raise Exception(f"Couldn't process document #{doc_idx}, sentence #{sent_idx}: expected pattern didn't match")

            # Process metadata
            if propagate_metadata:
                metadata_offset = 2 if process_paragraphs else 1
                metadata_fields = len(columns[metadata_offset:])
                current_metadata = '\t'.join(columns[metadata_offset:])

                if expected_metadata_fields >= 0 and metadata_fields != expected_metadata_fields:
                    needed_metadata_fields = expected_metadata_fields - metadata_fields

                    if needed_metadata_fields > 0:
                        logging.warning("Got %d metadata fields, but %d were expected: document %d, sentence %d: adding %d fake metadata fields",
                                        metadata_fields, expected_metadata_fields, doc_idx, sent_idx, needed_metadata_fields)

                        current_metadata += '\t' * (needed_metadata_fields - (1 if metadata_fields == 0 else 0))

                        if not current_metadata:
                            empty_but_valid_metadata = True
                    else:
                        logging.error("Got %d metadata fields, but %d were expected: document %d, sentence %d: %d unexpected metadata fields",
                                      metadata_fields, expected_metadata_fields, doc_idx, sent_idx, abs(needed_metadata_fields))

            paragraph = ''

            if len(sentences_wo_paragraphs) == 0:
                # Reason: skip == true or filter_trash() filtered out all the sentences
                if sent_idx == len(content) and content_not_processed:
                    # Any sentence from the document was processed
                    sentences_wo_paragraphs.append('') # Add "empty" sentence

                    fake_document = True
            else:
                content_not_processed = False

            # Process output
            for idx in range(len(sentences_wo_paragraphs)):
                # TODO total_sentences might be greater than len(sentences_wo_paragraphs) if any sentence was filtered out
                #   due to filter_trash and current_sentence_idx, in that case, might not be increased only in 1
                current_sentence_idx = idx + 1
                total_sentences = len(sentences_wo_paragraphs)

                if skip:
                    # Format of paragraph id.: p-1:-1s-1/-1
                    current_sentence_idx = -1
                    total_sentences = -1

                paragraph = f"p{paragraph_id}:{total_paragraphs}s{current_sentence_idx}/{total_sentences}" if process_paragraphs else ''
                sentences += f"{sentences_wo_paragraphs[idx]}"
                _current_metadata = paragraph + ('\t' if (current_metadata or empty_but_valid_metadata) and paragraph else '') + current_metadata
                metadata += f"{_current_metadata}\n"

                if same_output_file and not_only_text:
                    sentences += f"\t{_current_metadata}"

                if fake_document and not sentences:
                    pass # Empty document
                else:
                    sentences += '\n'

        no_sentences = sentences.count('\n') if not_only_text else -1 # Only calculated if metadata (purpose: sanity check)
        sentences = base64.b64encode(sentences.encode("utf-8"))

        if output_decode:
            writer.write(f"{sentences.decode('utf-8')}\n")
        else:
            writer.write(sentences + b'\n')

        if not_only_text and not same_output_file:
            no_metadata = metadata.count('\n')

            # Sanity checks
            if expected_metadata_fields >= 0:
                no_metadata_tabs = metadata.count('\t')
                no_metadata_tabs_expected = no_metadata * max(0, expected_metadata_fields - 1 + (1 if process_paragraphs else 0))

                if no_metadata_tabs != no_metadata_tabs_expected:
                    logging.error("Different number of tabs in metadata file: document %d: got %d, but %d were expected",
                                  doc_idx, no_metadata_tabs, no_metadata_tabs_expected)

            if no_sentences != no_metadata:
                if fake_document and no_sentences == 0 and no_metadata == 1:
                    pass # Special case
                else:
                    logging.error("Different number of lines in split sentences and metadata: document %d: %d vs %d",
                                  doc_idx, no_sentences, no_metadata)

            # Encode and write metadata
            metadata = base64.b64encode(metadata.encode("utf-8"))

            if metadata_decode:
                metadata_writer.write(f"{metadata.decode('utf-8')}\n")
            else:
                metadata_writer.write(metadata + b'\n')

if len(no_columns) > 1:
    logging.warning("Different number of columns were detected in the processed documents: %s", str(no_columns))
