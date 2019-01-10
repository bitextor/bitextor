#!/usr/bin/env python3

import sys
import base64
from boilerpipe.extract import Extractor

import argparse

oparser = argparse.ArgumentParser(description="Script that takes the output of bitextor-get-html-text.py -x and removes boilerplates from HTML.")
oparser.add_argument("-i", "--input", help="Path to the input file; if not defined, input is read from standard input", dest="input", default=None)
options = oparser.parse_args()

if options.input == None:
  reader = sys.stdin
else:
  reader = open(options.input,"r")

for line in reader:
  fields=line.strip().split("\t");
  if len(fields) == 5:
    try:
      content = base64.b64decode(fields[4]).decode("utf-8")
      extractor = Extractor(extractor='ArticleExtractor', html=content)
      extracted = extractor.getHTML()
      fields[4] = base64.b64encode(extracted.encode("utf-8")).decode("utf-8")
      print("\t".join(fields))
    except Exception as e:
      sys.stderr.write(str(e)+"\n")
