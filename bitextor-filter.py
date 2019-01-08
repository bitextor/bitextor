#!/usr/bin/env python3

#
# 1. This script reads a tab-separated list of documents, only containing two fields: the content of the document encoded with base64 and the URL
# 2. It decompress the base64 field and uses libmagic to identify the MIME type and character encoding
# 3. The output produced is:
#    character_encoding     MIME    URL    content_base64
#

import sys
import hashlib
import base64
import argparse


oparser = argparse.ArgumentParser(description="Script that takes the output of bitextor-crawl2ett and removes duplicate files.")
oparser.add_argument('--html-dir', dest='htmlDir', help='Directory containing files to compare')
oparser.add_argument('--text-dir', dest='textDir', help='Directory containing files in plain text')
oparser.add_argument("-l", "--languages", help="List accepted languages represented as a comma separated language codes list", dest="langlist", default=None)

options = oparser.parse_args()

langs=[]
if options.langlist != None:
  langs=options.langlist.strip().split(",")

seen_md5={}
for line in sys.stdin:
  pageToks = line.strip().split("\t")
  assert (len(pageToks) == 5)

  lang = pageToks[0]
  if lang in langs:
    htmlFile = open("{htmlDir}/{name}.html".format(htmlDir=options.htmlDir, name=pageToks[4]), "r")
    html_text = htmlFile.read()
    htmlFile.close()

    #We compute MD5 signature to compare files and detect duplicates
    c = hashlib.md5()
    c.update(html_text.encode("utf8"))
    #sys.stderr.write(c.hexdigest() + "\n")

    #checking for duplicate content (duplicates are discarded)
    if c.hexdigest() in seen_md5:
      pass
      #sys.stderr.write("Repeated file:\t"+pageToks[4]+"\tfirst occurrence\t"+seen_md5[c.hexdigest()]+"\n")
    else:
      textFile = open("{textDir}/{name}".format(textDir=options.textDir, name=pageToks[4]), "r")
      text = textFile.read()
      textFile.close()

      print("\t".join(pageToks[0:4])+"\t"+base64.b64encode(html_text.encode("utf-8")).decode("utf-8")+"\t"+base64.b64encode(text.encode("utf-8")).decode("utf-8")+"\n")
      seen_md5[c.hexdigest()]=pageToks[-1]
