#!/usr/bin/env python

import argparse
from argparse import RawTextHelpFormatter
import sys
from warcio.archiveiterator import ArchiveIterator
from warcio.warcwriter import WARCWriter

oparser = argparse.ArgumentParser(
    description="split - split a file into pieces\n\n"
                "Output pieces of FILE to PREFIXaa, PREFIXab, ...; default size is 1000 lines, and default PREFIX is 'x'.\n"
                "With no FILE, or when FILE is -, read standard input.", formatter_class=RawTextHelpFormatter)
oparser.add_argument('input', metavar='FILE', help='input WARC', nargs='?', default=sys.stdin)
oparser.add_argument('prefix', metavar='PREFIX', help='prefix of the file names', nargs='?', default="x")
oparser.add_argument("-r","--records", dest="NUMBER", default=1000,
                        help="put NUMBER records per output file")
oparser.add_argument("--verbose", action="store_true", default=False,
                        help="print a diagnostic just before each output file is opened")
options = oparser.parse_args()

if options.input is not sys.stdin:
    options.input = open(options.input,'rb')
else:
    options.input = sys.stdin.buffer


filecounter = 170
counter = 0
writer = None
for record in ArchiveIterator(options.input):
    if counter > int(options.NUMBER):
        counter = 0
    if counter == 0:
        if options.verbose:
            print("creating file '"+options.prefix+hex(filecounter)[2:]+".warc.gz"+"'")
        writer = WARCWriter(open(options.prefix+hex(filecounter)[2:]+".warc.gz",'wb'), gzip=True)
        filecounter += 1
    writer.write_record(record)
    counter += 1

