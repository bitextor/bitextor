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
oparser.add_argument('aligned_seg', metavar='FILE', nargs='?',
                     help='File containing the set of aliged segments (if undefined, the script reads from the '
                          'standard input)',
                     default=None)
oparser.add_argument("-s", "--stats", help="Print stats or just output the input", action="store_true",
                     dest="isPrintingStats", default=False)
oparser.add_argument("-f", "--filtering", help="Filter lines according to ELRC rules (printing stats required)",
                     action="store_true", dest="isFiltering", default=False)
oparser.add_argument("-c", "--columns",
                     help="Name of columns of the input tab separated file split by comma. Default: url1,url2,seg1,"
                          "seg2,hunalign,bicleaner",
                     default="url1,url2,seg1,seg2,hunalign,bicleaner")

options = oparser.parse_args()

if options.aligned_seg is not None:
    reader = open(options.aligned_seg, "r")
else:
    reader = sys.stdin

columns = options.columns.split(',')

for i in reader:
    fields = i.split("\t")
    fields[-1] = fields[-1].strip()
    fieldsdict = dict()
    extracolumns = []

    for field, column in zip(fields, columns):
        fieldsdict[column] = field
    if options.isPrintingStats:
        extracolumns = ["lengthratio", "numTokensSL", "numTokensTL"]
        if len(fieldsdict["seg2"]) == 0:
            lengthRatio = 0
        else:
            lengthRatio = len(fieldsdict["seg1"]) * 1.0 / len(fieldsdict["seg2"])
        numTokensSL = len(fieldsdict["seg1"].split(
            ' '))  # This is not the way this should be counted, we need to tokenize better first
        numTokensTL = len(fieldsdict["seg2"].split(
            ' '))  # This is not the way this should be counted, we need to tokenize better first
        fieldsdict["lengthratio"] = str(lengthRatio)
        fieldsdict["numTokensSL"] = str(numTokensSL)
        fieldsdict["numTokensTL"] = str(numTokensTL)
        if options.isFiltering:
            if "bicleaner" in fieldsdict and fieldsdict["bicleaner"].strip() != '':
                fieldsdict["bicleaner"] = str(round(float(fieldsdict["bicleaner"]), 4))
            if int(fieldsdict["numTokensSL"]) >= 200 or int(fieldsdict["numTokensTL"]) >= 200 or fieldsdict[
                "seg1"].strip() == '' or fieldsdict["seg2"].strip() == '' or float(
                    fieldsdict["lengthratio"]) >= 6 or float(fieldsdict["lengthratio"]) <= 0.1666:
                continue
    fieldstoprint = []
    for column in columns + extracolumns:
        fieldstoprint.append(fieldsdict[column])
    print("\t".join(fieldstoprint))
