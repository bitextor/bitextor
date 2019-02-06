#!/usr/bin/env python3

import sys
import warc
import argparse

oparser = argparse.ArgumentParser(
    description="Script that splits a WARC file into several WARC files by setting a maximum number of records")
oparser.add_argument('-o', '--output-dir', dest='outDir', help='Output directory', required=True)
oparser.add_argument('-m', '--max', dest='maxrecords', help='Language l1 in the crawl', default=10000, type=int)
options = oparser.parse_args()

recordcounter = 1
filecounter = 0
fout = warc.WARCFile(options.outDir + "/" + str(filecounter) + ".warc.gz", "w")

fin = warc.WARCFile(fileobj=sys.stdin.buffer)
for record in fin:
    fout.write_record(record)
    if recordcounter % options.maxrecords == 0:
        fout.close()
        filecounter += 1
        fout = warc.WARCFile(options.outDir + "/" + str(filecounter) + ".warc.gz", "w")
    recordcounter += 1
fout.close()
