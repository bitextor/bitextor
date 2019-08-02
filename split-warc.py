#!/usr/bin/env python

import argparse
from argparse import RawTextHelpFormatter
import sys
from warcio.archiveiterator import ArchiveIterator
from warcio.warcwriter import WARCWriter

A_UPPERCASE = ord('A')
ALPHABET_SIZE = 26


def _decompose(number):
    """Generate digits from `number` in base alphabet, least significants
    bits first.

    Since A is 1 rather than 0 in base alphabet, we are dealing with
    `number - 1` at each iteration to be able to extract the proper digits.
    """

    while number:
        number, remainder = divmod(number - 1, ALPHABET_SIZE)
        yield remainder


def base_10_to_alphabet(number):
    """Convert a decimal number to its base alphabet representation"""

    return ''.join(
        chr(A_UPPERCASE + part)
        for part in _decompose(number)
    )[::-1]


def base_alphabet_to_10(letters):
    """Convert an alphabet number to its decimal representation"""

    return sum(
        (ord(letter) - A_UPPERCASE + 1) * ALPHABET_SIZE ** i
        for i, letter in enumerate(reversed(letters.upper()))
    )


oparser = argparse.ArgumentParser(
    description="split - split a file into pieces\n\n"
                "Output pieces of FILE to PREFIXaa, PREFIXab, ...; "
                "default size is 1000 lines, and default PREFIX is 'x'.\n"
                "With no FILE, or when FILE is -, read standard input.", formatter_class=RawTextHelpFormatter)
oparser.add_argument('input', metavar='FILE', help='input WARC', nargs='?', default=sys.stdin)
oparser.add_argument('prefix', metavar='PREFIX', help='prefix of the file names', nargs='?', default="x")
oparser.add_argument("-r", "--records", dest="NUMBER", default=1000, help="put NUMBER records per output file")
oparser.add_argument("-a", "--suffix-length", dest="N", default=2, help="generate suffixes of length N (default 2)")
oparser.add_argument("--additional-suffix", dest="SUFFIX", default="", help="append an additional SUFFIX to file names")
oparser.add_argument("-d", dest="decimal", action="store_true", default=False,
                     help="use numeric suffixes starting at 0, not alphabetic")
oparser.add_argument("--numeric-suffixes", dest="FROM", help="same as -d, but allow setting the start value")
oparser.add_argument("-x", dest="hex", action="store_true", default=False,
                     help="use hex suffixes starting at 0, not alphabetic")
oparser.add_argument("--hex-suffixes", dest="FROMHEX", help="same as -x, but allow setting the start value")
oparser.add_argument("--verbose", action="store_true", default=False,
                     help="print a diagnostic just before each output file is opened")
options = oparser.parse_args()
if options.input is not sys.stdin:
    options.input = open(options.input, 'rb')
else:
    options.input = sys.stdin.buffer

if options.decimal or options.hex:
    filecounter = 0
elif options.FROM:
    filecounter = int(options.FROM)
elif options.FROMHEX:
    filecounter = int(options.FROMHEX, 16)
else:
    filecounter = 1 + (26 * (int(options.N) - 1))

counter = 0
writer = None
for record in ArchiveIterator(options.input):
    if counter > int(options.NUMBER):
        counter = 0
    if counter == 0:
        countersuffix = ""
        if options.decimal or options.FROM:
            countersuffix = str(filecounter).zfill(int(options.N))
        elif options.hex or options.FROMHEX:
            countersuffix = str(hex(filecounter)[2:]).zfill(int(options.N))
        else:
            countersuffix = base_10_to_alphabet(filecounter).lower()
        if options.verbose:
            print("creating file '" + options.prefix + countersuffix + options.SUFFIX + ".warc.gz" + "'")
        writer = WARCWriter(open(options.prefix + countersuffix + options.SUFFIX + ".warc.gz", 'wb'), gzip=True)
        filecounter += 1
    writer.write_record(record)
    counter += 1
