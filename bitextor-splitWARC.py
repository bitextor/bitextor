#!/usr/bin/env python3

import sys
import warc
import argparse

oparser = argparse.ArgumentParser(
    description="Script that splits a WARC file into several WARC files by setting a maximum number of records")
oparser.add_argument('-o', '--output-dir', dest='outDir', help='Output directory', required=True)
oparser.add_argument('-m', '--max', dest='maxrecords', help='Maximum number of records that a WARC file can contain; if 0 is set, a WARC file per record will be created', default=-1, type=int)
options = oparser.parse_args()

recordcounter = 1
filecounter = 0
fout = warc.WARCFile(options.outDir + "/" + str(filecounter) + ".warc.gz", "w")

fin = warc.WARCFile(fileobj=sys.stdin.buffer)
for record in fin:
    fout.write_record(record)
    if options.maxrecords >= 0 and recordcounter % options.maxrecords == 0:
        fout.close()
        filecounter += 1
        fout = warc.WARCFile(options.outDir + "/" + str(filecounter) + ".warc.gz", "w")
    recordcounter += 1
fout.close()
