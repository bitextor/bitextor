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
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../..")
from utils.common import open_xz_or_gzip_or_plain


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--text", dest="text_file", required=True,
                        help='File containing the sentence splitted plain text extracted from the HTML documents '
                             'in a WARC file, encoded in base64')
    parser.add_argument("--output_prefix", dest="output_prefix", default="", required=False,
                        help="Prefix for output files within directory")
    parser.add_argument("--prune", dest="prune_threshold", type=int, default=80,
                        help="Prune sentences longer than n (words/characters)", required=False)
    parser.add_argument("--prune_type", dest="prune_type", choices={"words", "chars"},
                        default="words", help="Prune sentences either by words or characters", required=False)
    args = parser.parse_args()

    counter = 0
    with open_xz_or_gzip_or_plain(args.text_file) as text_reader:
        for line in text_reader:
            counter = counter + 1
            text = base64.b64decode(line.strip()).decode("utf-8")
            if not text:
                continue
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
                print("{0}\t{1}".format(str(counter), extracted_line))
