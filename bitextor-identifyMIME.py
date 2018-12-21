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
oparser.add_argument('--in-dir', dest='inDir',
                    help='Directory of raw html files')

options = oparser.parse_args()

if options.crawl == None:
  reader = sys.stdin
else:
  reader = open(options.crawl,"r")

m=magic.open(magic.MAGIC_NONE)
m.load()

lineNum = 0
for line in reader:
  #sys.stderr.write("lineNum " + str(lineNum) + "\n")

  fields=line.strip().split("\t")
  if len(fields)>=2:
    url=fields[1]

    content=fields[0]

    #~Mime and encodign
    m.setflags(16|1024)
    #print("content", content)
    d = base64.b64decode(content + "==")
    #print("d", d)
    magicoutput=m.buffer(d).split(" ")
    magicoutput[0]=magicoutput[0][:-1]
    magicoutput.append(url)
    try:
      str1 = base64.b64encode(base64.b64decode(content + "==").decode(magicoutput[1].split("=")[1].replace("unknown-8bit","iso-8859-1").replace('us-ascii','iso-8859-1')).encode("utf8")).decode("utf8")
      magicoutput.append(str1)

      # write file
      #file = open("{inDir}/{name}.txt".format(inDir=options.inDir, name=lineNum), "r")
      #str2 = file.read()
      #file.close()
      #str2 = base64.b64encode(str2.encode()).decode()
      #magicoutput.append(str2)

      #sys.stderr.write("str1 " + str(len(str1)) + "\n")
      #sys.stderr.write("str2 " + str(len(str2)) + "\n")
      #sys.stderr.write("str1 " + str1 + "\n")
      #sys.stderr.write("str2 " + str2 + "\n")
      #sys.stderr.write("HH2\n")


      #sys.stderr.write(str)
    except:
      try:
        str1 = base64.b64encode(base64.b64decode(content + "==").decode('iso-8859-1').encode("utf8")).decode("utf8")
        magicoutput.append(str1)
      except:
        sys.stderr.write("Error in file " + url + "\n")
        magicoutput.append("ZHVubm8K") # dunno

    print("\t".join(magicoutput))

  else:
    sys.stderr.write("Wrong line: "+line.strip()+"\n")

  lineNum += 1
