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
import lzma
from sys import stdin

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--extracted_file", dest="sent_file", default='-',
                        help='File containing the translated text, URL<tab>sentence per line')
    args = parser.parse_args()

    if not args.sent_file or args.sent_file == '-':
        sent_reader = stdin
    else:
        sent_reader = lzma.open(args.sent_file, "rt")

    prevurl = ""
    prevtext = ""
    for line in sent_reader:
        line_split = line.strip().split('\t', 1)
        if len(line_split) == 0:
            continue
        elif len(line_split) == 1:
            url = line_split[0]
            text = ""
        else:
            url, text = line_split
        if url == prevurl:
            prevtext = prevtext + text + "\n"
        elif prevurl == "":
            prevurl = url
            prevtext = text + "\n"
        elif url != prevurl:
            print(f'{base64.b64encode(prevtext.encode()).decode()}')
            prevurl = url
            prevtext = text + "\n"
    print(f'{base64.b64encode(prevtext.encode()).decode()}')

    sent_reader.close()
