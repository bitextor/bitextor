#!/usr/bin/env python3

import warc
import base64
import sys
import argparse

parser = argparse.ArgumentParser(description='Extract html from warc files')
parser.add_argument('--out-dir', dest='outDir',
                    help='Directory to output html files')
args = parser.parse_args()
#print("outDir", args.outDir)

f = warc.WARCFile(fileobj=sys.stdin.buffer)

lineNum = 0
for record in f:
    text = base64.b64encode(record.payload.read()).decode('utf8')

    # write file
    file = open("{outDir}/{name}.txt".format(outDir=args.outDir, name=lineNum), "w")
    file.write(text)
    file.close()

    text = text.replace("\t"," ")
    print(text+"\t"+record.url+"\t"+record.date)

    lineNum += 1

file = open("{outDir}/count".format(outDir=args.outDir), "w")
file.write(str(lineNum) +  "\n")
file.close()
