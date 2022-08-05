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

#
# 1. All the alignments produced by bitextor-alignsegments are checked and those with an alignment confidence score
# lower than a given threshold are discarded (also those unaligned segments are discarded)
# 2. If, for a same document pair, a worng-alignment threshold is reached, the whole document pair is discarded
#
# Document final format:
# url1    url2    text1    text2
#

import sys
import argparse

oparser = argparse.ArgumentParser(
    description="Script that reads the output of bitextor-align-segments and cleans unaligned segments and segments "
                "with a confidence score too low")
oparser.add_argument('aligned_seg', metavar='FILE', nargs='?',
                     help='File containing the set of aliged segments provided by the script bitextor-align-segments '
                          '(if undefined, the script reads from the standard input)',
                     default=None)
oparser.add_argument("-q", "--min-quality",
                     help="Minimum reliability score for a pair of aligned segments to be considered acceptable (by "
                          "default, only pairs of segments with negative score are discarded)",
                     type=float, dest="minq", default=0)
oparser.add_argument("-m", "--max-lines",
                     help="Maximum number of segments which can be accepted as wrong alignments; if this number is "
                          "reached, the whole file pair is discarded",
                     type=int, dest="maxl", default=-1)
oparser.add_argument("-s", "--score", help="Print Hunalign score as a fifth column", action="store_true",
                     dest="isPrintingScore", default=False)

options = oparser.parse_args()

if options.aligned_seg is not None:
    reader = open(options.aligned_seg, "r")
else:
    reader = sys.stdin

last_filepair = (None, None)
error_count = 0
segment_list = []
for i in reader:
    campos = i.strip().split("\t")
    if len(campos) >= 5:
        if last_filepair[0] != campos[0] or last_filepair[1] != campos[1]:
            if options.maxl == -1 or error_count < options.maxl:
                if len(segment_list) > 0:
                    print("\n".join(segment_list))
            error_count = 0
            last_filepair = (campos[0], campos[1])
            segment_list = []
        if float(campos[4]) > options.minq:
            if not options.isPrintingScore:
                campos.pop(4)
            segment_list.append("\t".join(campos))
        else:
            # sys.stderr.write("CONFIDENCE SCORE TOO LOW FOR PAIR OF SEGMENTS: " + i)
            error_count = error_count + 1
    else:
        # sys.stderr.write("UNALIGNED SEGMENT: " + i)
        pass

# reader.close()
if len(segment_list) > 0:
    print("\n".join(segment_list))
