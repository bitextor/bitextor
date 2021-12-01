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
import shlex
import base64
import logging
import argparse
import contextlib
import subprocess

def join(input_file='-', separator='\t', join_str='\t', is_plaintext=False):
    if separator == '\n':
        raise Exception(f"the provided separator is not valid: '{repr(separator)}'")

    cm = contextlib.nullcontext(sys.stdin)
    columns = set()

    if input_file != '-':
        cm = open(input_file)

    with cm as doc_fd:
        for idx, doc in enumerate(doc_fd):
            try:
                doc = doc.strip('\n').split(separator)
                segments = []
                text = []

                # Get all the segments from the current document
                for segment in doc:
                    if not is_plaintext:
                        segment = base64.b64decode(segment).decode('utf-8')

                    segments.append(segment)

                columns.add(len(segments))
                nosentences = set()
                sentences_from_segments = []

                # Get all the sentences from all the segments
                for segment in segments:
                    if len(segment) > 0 and segment[-1] == '\n':
                        segment = segment[:-1]

                    sentences = segment.split('\n')

                    sentences_from_segments.append(sentences)
                    nosentences.add(len(sentences))

                if len(nosentences) not in (0, 1):
                    raise Exception(f"document #{idx + 1}: same number of sentences were expected, but got different")

                # Join all the sentences from the segments for the current document
                if len(nosentences) == 1:
                    nosentences = nosentences.pop()

                    for i in range(nosentences):
                        current_sentence = join_str.join([sentences_from_segments[j][i] for j in range(len(sentences_from_segments))])

                        text.append(current_sentence)


                doc = '\n'.join(text).encode('utf-8')
                doc = base64.b64encode(doc).decode('utf-8')

                print(doc)
            except Exception as e:
                raise Exception(f"document #{idx + 1} could not be processed") from e

    if len(columns) not in (0, 1):
        logging.warning("different number of segments (columns) were processed: %s", columns)

def parse_args():
    parser = argparse.ArgumentParser(description="It joins the sentences of the content from BASE64 documents and returns a new BASE64-encoded document",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--input', default='-',
                        help="Input file. If '-' is provided, stdin will be used")
    parser.add_argument('--input-is-not-base64', action='store_true',
                        help="Input is expected to be BASE64-encoded text, but with this option plaintext will be expected instead")
    parser.add_argument('--separator', default='\t',
                        help="Separator which will be used to split the initial input")
    parser.add_argument('--join-str', default='\t',
                        help="String which will be used to join the BASE64-decoded sentences before encode the document again")
    parser.add_argument('--logging-level', type=int, default=logging.INFO,
                        help="Logging level")

    args = parser.parse_args()

    return args

if __name__ == '__main__':
    args = parse_args()

    logging.basicConfig(level=args.logging_level)

    join(input_file=args.input, separator=args.separator, join_str=args.join_str, is_plaintext=args.input_is_not_base64)
