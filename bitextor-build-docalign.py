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
import os
import lzma
import gzip

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from utils.common import open_xz_or_gzip_or_plain

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--indices', dest='indices', required=True,
                        help='Output of the document aligner: pairs of document indices')
    parser.add_argument('--text1', help='Path to the file with the plain text of the documents in LANG1', required=True)
    parser.add_argument('--tokenized1', required=True,
                        help='Path to the file with the tokenized text of the documents crawled in LANG1')
    parser.add_argument('--text2', help='Path to the file with the plain text of the documents in LANG2', required=True)
    parser.add_argument('--tokenized2', required=True,
                        help='Path to the file with the tokenized text of the documents crawled in LANG2')
    parser.add_argument('--column1', help='Column that contains the first document of the document pairs',
                        default=0, type=int)
    parser.add_argument('--column2', help='Column that contains the second document of the document pairs',
                        default=1, type=int)

    args = parser.parse_args()

    lang2_docs = set()
    lang2_read_docs = {}

    if args.indices[:-3] == '.xz':
        reader = lzma.open(args.indices, 'rt')
    elif args.indices[:-3] == '.gz':
        reader = gzip.open(args.indices, 'rt')
    else:
        reader = open(args.indices, 'r')

    for line in reader:
        fields = line.split('\t')
        lang2_docs.add(int(fields[args.column2]))

    reader.seek(0)

    with open_xz_or_gzip_or_plain(args.tokenized1) as tok_reader1, \
            open_xz_or_gzip_or_plain(args.tokenized2) as tok_reader2, \
            open_xz_or_gzip_or_plain(args.text1) as text_reader1, \
            open_xz_or_gzip_or_plain(args.text2) as text_reader2:

        doc1_current_line = 1
        doc2_current_line = 1
        doc2_last_written = 0

        for line in reader:
            fields = line.strip().split('\t')
            doc1 = int(fields[args.column1])
            doc2 = int(fields[args.column2])
            while doc1_current_line <= doc1:
                text1 = next(text_reader1, None).strip()
                tok1 = next(tok_reader1, None).strip()
                doc1_current_line = doc1_current_line + 1

            while doc2_last_written != doc2:
                if doc2_current_line <= doc2:
                    text2 = next(text_reader2, None).strip()
                    tok2 = next(tok_reader2, None).strip()

                    if doc2_current_line == doc2:
                        print("{0}\t{1}\t{2}\t{3}\t{4}\t{5}".format(doc1, doc2, text1, text2, tok1, tok2))
                        doc2_last_written = doc2
                    elif doc2_current_line in lang2_docs:
                        lang2_read_docs[doc2_current_line] = (text2, tok2)
                        lang2_docs.remove(doc2_current_line)

                    doc2_current_line = doc2_current_line + 1

                if doc2 in lang2_read_docs:
                    text2, tok2 = lang2_read_docs[doc2]
                    print("{0}\t{1}\t{2}\t{3}\t{4}\t{5}".format(doc1, doc2, text1, text2, tok1, tok2))
                    del lang2_read_docs[doc2]
                    doc2_last_written = doc2
