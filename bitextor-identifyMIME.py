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
oparser.add_argument('crawl', metavar='CRAWL', nargs='?', help='Output of the bitextor-crawl script that provides a tab-separated list of documents, only containing two fields: the content of the document encoded with base64 and the URL.', default=None)
options = oparser.parse_args()

if options.crawl == None:
  reader = sys.stdin
else:
  reader = open(options.crawl,"r")

m=magic.open(magic.MAGIC_NONE)
m.load()
for line in reader:
  fields=line.strip().split("\t")
  if len(fields)>=2:
    url=fields[1]
    content=fields[0]
    #~Mime and encodign
    m.setflags(16|1024)
    magicoutput=m.buffer(base64.b64decode(content)).split(" ")
    magicoutput[0]=magicoutput[0][:-1]
    magicoutput.append(url)
    try:
      magicoutput.append(base64.b64encode(base64.b64decode(content).decode(magicoutput[1].split("=")[1].replace("unknown-8bit","iso-8859-1").replace('us-ascii','iso-8859-1')).encode("utf8")).decode("utf8"))
      print("\t".join(magicoutput))
    except LookupError as e:
      sys.stderr.write("Unknown character encoding in file "+url+": "+str(e)+"\n")
  else:
    sys.stderr.write("Wrong line: "+line.strip()+"\n")
