#!/usr/bin/env python3

#
# 1. This script reads a tab-separated list of documents, only containing two fields: the content of the document encoded with base64 and the URL
# 2. It decompress the base64 field and uses libmagic to identify the MIME type and character encoding
# 3. The output produced is:
#    character_encoding     MIME    URL    content_base64
#

import sys
import magic
import base64
import argparse

oparser = argparse.ArgumentParser(description="Script that takes the output of bitextor-crawl and adds to the list of fields the MIME type and the character encoding detected.")
oparser.add_argument('--in-dir', dest='inDir', help='Directory of raw html files')

options = oparser.parse_args()

m=magic.open(magic.MAGIC_NONE)
m.load()
#sys.stderr.write("m:" + str(m) + "\n")

with open("{inDir}/page".format(inDir=options.inDir), "rt") as pageFile:
  pages = pageFile.read().strip().split("\n")
  #sys.stderr.write("pages:" + str(len(pages)) + " " + str(pages) + "\n")


lineNum = 0
for line in pages:
  #sys.stderr.write("lineNum " + str(lineNum) + "\n")

  fields=line.strip().split("\t")
  if len(fields)>=1:
    url=fields[0]

    #~Mime and encodign
    m.setflags(16|1024)

    # read file
    file = open("{inDir}/{name}".format(inDir=options.inDir, name=lineNum), "r")
    text = file.read()
    file.close()
    #sys.stderr.write("text " + str(type(text)) + "\n")

    mimeEncode = m.buffer(text.encode()).split(" ")
    mimeEncode[0] = mimeEncode[0][:-1]
    #sys.stderr.write("mimeEncode:" + str(mimeEncode) + "\n")

    #magicoutput = mimeEncode
    magicoutput = []
    magicoutput.append(url)
    #sys.stderr.write("magicoutput:" + str(magicoutput) + "\n")

    pages[lineNum] = line + "\t" + mimeEncode[0] + "\t" + mimeEncode[1]

  else:
    sys.stderr.write("Wrong line: "+line.strip()+"\n")

  lineNum += 1

with open("{inDir}/page".format(inDir=options.inDir), "wt") as pageFile:
  pageFile.write("\n".join(pages))
  pageFile.write("\n")
