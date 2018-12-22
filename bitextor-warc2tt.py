#!/usr/bin/env python3

import warc
import base64
import sys
import argparse
import cchardet

#############################################################################
def convert_encoding(data, new_coding = 'UTF-8'):
  encoding = cchardet.detect(data)['encoding']

  if new_coding.upper() != encoding.upper():
    #sys.stderr.write("convert " + encoding + " to " + new_coding + "\n")
    data = data.decode(encoding).encode(new_coding)

  #print("data", type(data))
  return data

#############################################################################

parser = argparse.ArgumentParser(description='Extract html from warc files')
parser.add_argument('--out-dir', dest='outDir',
                    help='Directory to output html files')
args = parser.parse_args()
#print("outDir", args.outDir)

f = warc.WARCFile(fileobj=sys.stdin.buffer)

lineNum = 0
for record in f:
    text = record.payload.read() #.decode('utf8')
    text = convert_encoding(text)

    # write file
    file = open("{outDir}/{name}.txt".format(outDir=args.outDir, name=lineNum), "w")
    file.write(text.decode())
    file.close()

    print(record.url+"\t"+record.date)

    lineNum += 1

file = open("{outDir}/count".format(outDir=args.outDir), "w")
file.write(str(lineNum) +  "\n")
file.close()
