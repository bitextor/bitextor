#!__ENV__ __PYTHON__

#
# 1. All the alignments produced by bitextor-alignsegments are checked and those with an alignemnt confidence score lower than a given threshold are discarded (also those unaligned segments are discarded)
# 2. If, for a same document pair, a worng-alignment threshold is reached, the whole document pair is discarded
#
# Formato final del documento:
# url1    url2    text1    text2
#

import sys
import argparse

oparser = argparse.ArgumentParser(description="Script that reads the output of bicleaner_classifier_full.py and filters out segments with a score lower than a given threshold.")
oparser.add_argument('bicleaner_file', metavar='FILE', nargs='?', help='File containing the output of bicleaner (if undefined, the script reads from the standard input)', default=None)
oparser.add_argument('--threshold', type=float, default=0.5, help="Threshold for bicleaner score.")

options = oparser.parse_args()

if options.bicleaner_file != None:
  reader = open(options.bicleaner_file,"r")
else:
  reader = sys.stdin

for line in reader:
  parts = line.split('\t')
  parts[-1]=parts[-1].strip()
  if float(parts[-1]) >= float(options.threshold):
    sys.stdout.write(line)

