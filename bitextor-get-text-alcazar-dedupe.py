#!/usr/bin/env python3

#
# Input is an xtt file with:
# text/html \t charset=utf8 \t URL \t html \t normed-html
#
# Output is:
# first three field, plus extracted text (twice)
# Q. Is fourth field required?
#

import sys
import hashlib
import base64
import argparse

import alcazar.bodytext

oparser = argparse.ArgumentParser(description="Script that extracts text from html and dedupes")
oparser.add_argument('xtt', metavar='XTT', nargs='?', help='Output of bitextor-get-html-text.py (in format XTT).', default=None)
options = oparser.parse_args()

if options.xtt == None:
  reader = sys.stdin
else:
  reader = open(options.ett,"r")

seen_md5={}
for i in reader:
  fields = i.strip().split("\t")
  try:
    html = base64.b64decode(fields[4]).decode("utf-8");
    text = alcazar.bodytext.parse_article(html)
    if not text.body_text:
      continue
    #We compute MD5 signature to compare files and detect duplicates
    c = hashlib.md5()
    c.update(text.body_text.encode("utf8"))
    if c.hexdigest() in seen_md5:
      continue
    seen_md5[c.hexdigest()]=fields[2]
    text = base64.b64encode(text.body_text.replace("\t", " ").encode("utf-8")).decode("utf-8")
    print("{0}\t{1}\t{2}\t{3}\t{4}".format(fields[0].strip(),fields[1],fields[2],fields[4],text))
  except UnicodeDecodeError:
    #sys.stderr.write("File "+fields[2]+" produced a character encoding error")
    pass
