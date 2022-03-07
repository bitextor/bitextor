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
import argparse

oparser = argparse.ArgumentParser(
    description="Script that reads the output of bicleaner and filters out segments with a score lower than a given "
                "threshold.")

oparser.add_argument("bicleaner_file", metavar="FILE", nargs="?",
                     help="File containing the output of bicleaner (if undefined, the script reads from the standard "
                          "input)")
oparser.add_argument('--header-field', default="bicleaner_score",
                     help="Header field which it will be used to get the column where the bicleaner score is")
oparser.add_argument('--threshold', type=float, default=0.5, help="Threshold for bicleaner score.")

options = oparser.parse_args()

if options.bicleaner_file is not None:
    reader = open(options.bicleaner_file, "r")
else:
    reader = sys.stdin

header = next(reader).strip().split('\t')

if options.header_field not in header:
    raise Exception(f"The header field '{options.header_field}' was expected but was not found")

sys.stdout.write('\t'.join(header) + '\n')

bicleaner_score_idx = header.index(options.header_field)

for line in reader:
    parts = line.split('\t')
    bicleaner_score = parts[bicleaner_score_idx].strip()

    if float(bicleaner_score) >= float(options.threshold):
        sys.stdout.write(line)
