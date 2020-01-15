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

import argparse
import base64
import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../utils")
from common import open_xz_or_gzip_or_plain

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--sentences_file", dest="sent_file", required=True,
                        help='File containing the sentence splitted plain text extracted from the HTML documents '
                             'in a WARC file, encoded in base64')
    parser.add_argument("--tokenized_file", dest="tok_file", required=False, default="",
                        help="File containing the sentence splitted tokenized text extracted from the HTML documents")
    parser.add_argument("--extracted_output", dest="out_extracted", default="extracted.xz",
                        help='Path of the output file that will contain extracted sentences')
    parser.add_argument("--tokenized_output", dest="out_tokenized", default="tokenized.xz",
                        help='Path of the outupt file that will contain tokenized sentences')
    parser.add_argument("--output_prefix", dest="output_prefix", default="", required=False,
                        help="Prefix for output files within directory")
    parser.add_argument("--prune", dest="prune_threshold", type=int, default=80,
                        help="Prune sentences longer than n (words/characters)", required=False)
    parser.add_argument("--prune_type", dest="prune_type", choices={"words", "chars"},
                        default="words", help="Prune sentences either by words or characters", required=False)
    args = parser.parse_args()

    counter = 0

    if args.tok_file:
        with open_xz_or_gzip_or_plain(args.sent_file) as sent_reader, open_xz_or_gzip_or_plain(args.tok_file) as tok_reader, \
                open_xz_or_gzip_or_plain(args.out_extracted, "wt") as sent_writer, open_xz_or_gzip_or_plain(args.out_tokenized, "wt") as tok_writer:
            for sent_doc in sent_reader:
                counter = counter + 1
                tok_doc = next(tok_reader, None)
                sent_text = base64.b64decode(sent_doc.strip()).decode("utf-8")
                tok_text = base64.b64decode(tok_doc.strip()).decode("utf-8")

                for line in list(zip(sent_text.split("\n"), tok_text.split("\n"))):
                    sent_line = line[0].strip()
                    tok_line = line[1].strip()

                    if not sent_line or not tok_line:
                        continue

                    # prune long sentences
                    if args.prune_type == "chars":
                        if len(sent_line) > args.prune_threshold:
                            continue
                    elif args.prune_type == "words":
                        if len(sent_line.split()) > args.prune_threshold:
                            continue

                    sent_writer.write("{}\t{}\n".format(str(counter), sent_line))
                    tok_writer.write("{}\t{}\n".format(str(counter), tok_line))

    else:
        with open_xz_or_gzip_or_plain(args.sent_file) as sent_reader, open_xz_or_gzip_or_plain(args.out_extracted, "wt") as sent_writer:
            for line in sent_reader:
                counter = counter + 1
                text = base64.b64decode(line.strip()).decode("utf-8")

                for extracted_line in text.split("\n"):
                    extracted_line = extracted_line.strip()

                    if not extracted_line:
                        continue

                    # prune long sentences
                    if args.prune_type == "chars":
                        if len(extracted_line) > args.prune_threshold:
                            continue
                    elif args.prune_type == "words":
                        if len(extracted_line.split()) > args.prune_threshold:
                            continue
                    sent_writer.write("{}\t{}\n".format(str(counter), extracted_line))
