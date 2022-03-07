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
    parser.add_argument('--columns1-output-headers', nargs='+', required=True)
    parser.add_argument('--columns2-output-headers', nargs='+', required=True)

    args = parser.parse_args()

    if len(args.lang1_column_filename) != len(args.columns1_output_headers):
        raise Exception("Different number of elements for --columns1 and --columns1-output-headers "
                        f"({args.lang1_column_filename} vs {args.columns1_output_headers})")
    if len(args.lang2_column_filename) != len(args.columns2_output_headers):
        raise Exception("Different number of elements for --columns2 and --columns2-output-headers "
                        f"({args.lang2_column_filename} vs {args.columns2_output_headers})")

    lang2_docs = set()
    lang2_read_docs = {}
    indices = list()

    if not args.indices:
        args.indices = '-'

    with open_xz_or_gzip_or_plain(args.indices) if args.indices != '-' else sys.stdin as reader:
        header = next(reader).strip().split("\t")
        src_doc_idx_idx = header.index("src_index")
        trg_doc_idx_idx = header.index("trg_index")

        for line in reader:
            fields = line.strip().split('\t')
            src_doc_idx = int(fields[src_doc_idx_idx])
            trg_doc_idx = int(fields[trg_doc_idx_idx])

            lang2_docs.add(trg_doc_idx)
            indices.append((src_doc_idx, trg_doc_idx))

    readers1 = [open_xz_or_gzip(filename, 'rt') for filename in args.lang1_column_filename]
    readers2 = [open_xz_or_gzip(filename, 'rt') for filename in args.lang2_column_filename]

    doc1_current_line = 1
    doc2_current_line = 1
    data1 = []
    data2 = []

    tab = "\t"

    # Print output header
    sys.stdout.write(f"src_index\ttrg_index")

    for c1, c2 in zip(args.columns1_output_headers, args.columns2_output_headers):
        sys.stdout.write(f"\t{c1}\t{c2}")

    sys.stdout.write('\n')

    for doc1, doc2 in indices:
        while doc1_current_line <= doc1:
            data1 = [next(reader, None).strip() for reader in readers1]
            doc1_current_line = doc1_current_line + 1

        if doc2_current_line <= doc2:
            while doc2_current_line <= doc2:
                data2 = [next(reader, None).strip() for reader in readers2]

                if doc2_current_line == doc2:
                    sys.stdout.write(f"{doc1}\t{doc2}")

                    for d1, d2 in zip(data1, data2):
                        sys.stdout.write(f"\t{d1}\t{d2}")

                    sys.stdout.write('\n')
                elif doc2_current_line in lang2_docs:
                    lang2_read_docs[doc2_current_line] = data2

                doc2_current_line = doc2_current_line + 1
        else:
            data2 = lang2_read_docs[doc2]

            sys.stdout.write(f"{doc1}\t{doc2}")

            for d1, d2 in zip(data1, data2):
                sys.stdout.write(f"\t{d1}\t{d2}")

            sys.stdout.write('\n')

    for r in readers1:
        r.close()

    for r in readers2:
        r.close()
