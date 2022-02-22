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
    description="Script that reads takes a list of aligned segments, such as that produced by bitextor-alignsegments "
                "script, and computes the basic ELRC quality metrics: number of tokens in lang1/lang2 and length "
                "ratio.")

oparser.add_argument("aligned_seg", metavar="FILE", nargs="?",
                     help="File containing the set of aliged segments (if undefined, the script reads from the "
                          "standard input)")
oparser.add_argument("-s", "--stats", action="store_true", dest="isPrintingStats",
                     help="Print stats or just output the input")
oparser.add_argument("-f", "--filtering", action="store_true", dest="isFiltering",
                     help="Filter lines according to ELRC rules (printing stats required)")

options = oparser.parse_args()

if options.aligned_seg is not None:
    reader = open(options.aligned_seg, "r")
else:
    reader = sys.stdin

header = next(reader).strip().split('\t')
extra_columns = []

if options.isPrintingStats:
    extra_columns = ["length_ratio", "src_num_tokens", "trg_num_tokens"]

# Print output header
print('\t'.join(header + extra_columns))

for i in reader:
    if options.isPrintingStats:
        fields = i.split('\t')
        fields[-1] = fields[-1].strip()
        fieldsdict = dict()

        # Create dict with the header to work the values easily
        for field, column in zip(fields, header):
            fieldsdict[column] = field

        lengthRatio = 0 if fieldsdict["trg_text"] == '' else len(fieldsdict["src_text"]) / len(fieldsdict["trg_text"])
        numTokensSL = len(fieldsdict["src_text"].split(' ')) # We should tokenize better
        numTokensTL = len(fieldsdict["trg_text"].split(' ')) # We should tokenize better
        fieldsdict["length_ratio"] = str(lengthRatio)
        fieldsdict["src_num_tokens"] = str(numTokensSL)
        fieldsdict["trg_num_tokens"] = str(numTokensTL)

        if options.isFiltering:
            if "bicleaner_score" in fieldsdict:
                fieldsdict["bicleaner_score"] = str(round(float(fieldsdict["bicleaner_score"]), 4))
            elif "bicleaner_ai_score" in fieldsdict:
                fieldsdict["bicleaner_ai_score"] = str(round(float(fieldsdict["bicleaner_ai_score"]), 4))

            if int(fieldsdict["src_num_tokens"]) >= 200 or int(fieldsdict["trg_num_tokens"]) >= 200:
                continue
            if fieldsdict["src_text"].strip() == '' or fieldsdict["trg_text"].strip() == '':
                continue
            if float(fieldsdict["length_ratio"]) <= 0.1666 or float(fieldsdict["length_ratio"]) >= 6:
                continue

        fieldstoprint = []

        for column in header + extra_columns:
            fieldstoprint.append(fieldsdict[column])

        print('\t'.join(fieldstoprint))
    else:
        sys.stdout.write(i)
