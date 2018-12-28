#!/usr/bin/env python3
#
# 1. Read lines from .ett file
# 2. For eac line, the HTML is cleaned and the language is detected for the raw text
# 3. Output is printed following the format:
#
# language	encoding	mimetype	url	content(base_64)
#
#

import sys
import base64
from html.parser import HTMLParser
import langid
import pycld2 as cld2
import argparse
import socket
import re


def guess_lang_from_data2(data):
  reliable, text_bytes, detected_languages = cld2.detect(
    data, isPlainText=False)
  #print("detected_languages", detected_languages)
  return detected_languages[0][1]


oparser = argparse.ArgumentParser(description="Script that reads the output of bitextor-webdir2ett and, for each line (lines correspond to files in de website) the language of the document is detected and this information is added to the information about the documents.")
oparser.add_argument("ett_path", metavar="FILE", nargs="?", help="File containing the output of bitextor-webdir2ett (if undefined, the script reads from the standard input)", default=None)
oparser.add_argument("-l", "--languages", help="List accepted languages represented as a comma separated language codes list", dest="langlist", default=None)
oparser.add_argument('--root-dir', dest='rootDir', help='Domain directory')
options = oparser.parse_args()

langs=[]
if options.langlist != None:
  langs=options.langlist.strip().split(",")

if options.ett_path != None:
  reader = open(options.ett_path,"r")
else:
  reader = sys.stdin

#Reading line by line from the standard output
for line in reader:
  linefields=line.strip().split("\t")
  assert(len(linefields) == 6)

  lineNum = linefields[5]
  #sys.stderr.write(lineNum + "\n")

  deboiledFile = open("{rootDir}/deboiled/{name}".format(rootDir=options.rootDir, name=lineNum), "r")
  html_text = deboiledFile.read()
  deboiledFile.close()

  if len(html_text)>0:
    #detecting language
    #lang, conf = langid.classify(parsed_text)
    lang = guess_lang_from_data2(html_text)

    if len(langs)==0 or lang in langs:
      textFile = open("{rootDir}/text/{name}".format(rootDir=options.rootDir, name=lineNum), "rt")
      parsed_text = textFile.read()
      textFile.close()

      e = base64.b64encode(parsed_text.replace("\t", " ").encode("utf-8")).decode("utf8")

      outFields = [lang,
                   linefields[0],
                   linefields[1],
                   linefields[2],
                   linefields[3],
                   e]

      print("\t".join(outFields))

