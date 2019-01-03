#!/usr/bin/env python3

import warc
import base64
import sys
import argparse
import cchardet

#############################################################################
def convert_encoding(data, new_coding = 'UTF-8'):
  encoding = cchardet.detect(data)['encoding']
  #sys.stderr.write("convert " + encoding + " to " + new_coding + "\n")

  if new_coding.upper() != encoding.upper():
    #sys.stderr.write("convert " + encoding + " to " + new_coding + "\n")
    data = data.decode(encoding).encode(new_coding)

  #print("data", type(data))
  return data

#############################################################################

parser = argparse.ArgumentParser(description='Extract html from warc files')
parser.add_argument('--root-dir', dest='rootDir', help='Domain directory')
args = parser.parse_args()
#print("outDir", args.outDir)

pageFile = open("{rootDir}/page".format(rootDir=args.rootDir), "w")

f = warc.WARCFile(fileobj=sys.stdin.buffer)

lineNum = 0
for record in f:
    #sys.stderr.write("lineNum " + str(lineNum) \
    #                 + " " + record.url \
    #                 + " " + record.date + "\n")

    text = record.payload.read() #.decode('utf8')
    #sys.stderr.write("text" + str(len(text)) + "\n")

    if len(text) > 0:
        text = convert_encoding(text)

        # write file
        file = open("{rootDir}/raw-html/{name}".format(rootDir=args.rootDir, name=lineNum), "w")
        file.write(text.decode())
        file.close()

        pageFile.write(record.url + "\t" + record.date + "\n")

        lineNum += 1

pageFile.close()
