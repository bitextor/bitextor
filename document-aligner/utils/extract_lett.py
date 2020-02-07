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
    parser.add_argument("--sentences_file", dest="sent_file", default='-',
                        help='File containing the sentence splitted plain text extracted from the HTML documents '
                             'in a WARC file, encoded in base64')
    args = parser.parse_args()

    counter = 0

    if not args.sent_file:
        args.sent_file = '-'

    with open_xz_or_gzip_or_plain(args.sent_file) if args.sent_file != '-' else sys.stdin as sent_reader:
        for line in sent_reader:
            counter = counter + 1
            text = base64.b64decode(line.strip()).decode("utf-8")
            n_lines = 0

            for extracted_line in text.split("\n"):
                extracted_line = extracted_line.strip()
                if not extracted_line:
                    continue
                n_lines = n_lines + 1
                print(f'{counter}\t{extracted_line}')

            if n_lines == 0:
                print(f'{counter}\t ')  # maintain the number of documents of the original file
