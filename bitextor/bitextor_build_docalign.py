#!/usr/bin/env python


#  This file is part of Bitextor.
#
#  Bitextor is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bitextor is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bitextor.  If not, see <https://www.gnu.org/licenses/>.

import argparse
import sys
import gzip
import lzma

from bitextor.utils.common import open_xz_or_gzip_or_plain

def open_xz_or_gzip(filename, mode='rt'):
    if filename[-3:] == '.xz':
        return lzma.open(filename, mode)
    elif filename[-3:] == '.gz':
        return gzip.open(filename, mode)
    else:
        return open(filename, mode)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Provide pair of document indices (line numbers)'
                                                 ' and find data (columns) corresponding to these'
                                                 ' documents in line based format. '
                                                 'Output format will be: '
                                                 'INDEX1<tab>[COLUMNS1...]<tab>[INDEX2]<tab>[COLUMNS2...]')
    parser.add_argument('--indices', dest='indices', default='-',
                        help='pairs of document indices, sorted by first column')
    parser.add_argument('--columns1', dest='lang1_column_filename', nargs='+', required=True)
    parser.add_argument('--columns2', dest='lang2_column_filename', nargs='+', required=True)

    args = parser.parse_args()

    lang2_docs = set()
    lang2_read_docs = {}
    indices = list()

    if not args.indices:
        args.indices = '-'

    with open_xz_or_gzip_or_plain(args.indices) if args.indices != '-' else sys.stdin as reader:
        for line in reader:
            fields = line.strip().split('\t')
            lang2_docs.add(int(fields[1]))
            indices.append((int(fields[0]), int(fields[1])))

    readers1 = [open_xz_or_gzip(filename, 'rt') for filename in args.lang1_column_filename]
    readers2 = [open_xz_or_gzip(filename, 'rt') for filename in args.lang2_column_filename]

    doc1_current_line = 1
    doc2_current_line = 1
    data1 = []
    data2 = []

    tab = "\t"

    for doc1, doc2 in indices:
        while doc1_current_line <= doc1:
            data1 = [next(reader, None).strip() for reader in readers1]
            doc1_current_line = doc1_current_line + 1
        if doc2_current_line <= doc2:
            while doc2_current_line <= doc2:
                data2 = [next(reader, None).strip() for reader in readers2]
                if doc2_current_line == doc2:
                    print(f'{doc1}\t{tab.join(data1)}\t{doc2}\t{tab.join(data2)}')
                elif doc2_current_line in lang2_docs:
                    lang2_read_docs[doc2_current_line] = data2
                doc2_current_line = doc2_current_line + 1
        elif doc2_current_line > doc2:
            data2 = lang2_read_docs[doc2]
            print(f'{doc1}\t{tab.join(data1)}\t{doc2}\t{tab.join(data2)}')
            del lang2_read_docs[doc2] # so far document aligner doesn't produce repeated indices, so deleting entries is safe

    for r in readers1:
        r.close()

    for r in readers2:
        r.close()
