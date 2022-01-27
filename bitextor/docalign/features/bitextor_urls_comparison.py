#!/usr/bin/env python3

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

import sys
import argparse
import Levenshtein
import re

from bitextor.utils.common import open_xz_or_gzip_or_plain


def read_urls(f, docs, fileid):
    with open_xz_or_gzip_or_plain(f) as fd:
        for u in fd:
            u = u.strip()
            rx = re.match('(https?://[^/:]+)', u)
            if rx is not None:
                url_domain = rx.group(1)
                url = u.replace(url_domain, "")
            else:
                url = u
            docs[fileid] = url
            fileid += 1
    return fileid


def main():
    oparser = argparse.ArgumentParser(
        description="Script that rescores the aligned-document candidates provided by script bitextor-idx2ridx by using "
                    "the Levenshtein edit distance of the structure of the files.")
    oparser.add_argument('ridx', metavar='RIDX', nargs='?', default=None,
                         help='File with extension .ridx (reverse index) from bitextor-idx2ridx (if not provided, '
                         'the script will read from the standard input)')
    oparser.add_argument("--url1", help="File produced during pre-processing containing all the URLs in a WARC file for SL",
                         dest="url1", required=True)
    oparser.add_argument("--url2", help="File produced during pre-processing containing all the URLs in a WARC file for TL",
                         dest="url2", required=True)
    options = oparser.parse_args()

    if options.ridx is None:
        reader = sys.stdin
    else:
        reader = open(options.ridx, "r")

    documents = {}
    offset = 1
    offset = read_urls(options.url1, documents, offset)
    offset = read_urls(options.url2, documents, offset)

    header = next(reader).strip().split("\t")
    src_doc_idx_idx = header.index("src_index")
    trg_doc_idx_idx = header.index("trg_index")

    # Print output header
    print("\t".join(header) + "\turls_distance")

    for i in reader:
        fields = i.strip().split("\t")
        src_doc_idx = int(fields[src_doc_idx_idx])
        trg_doc_idx = int(fields[trg_doc_idx_idx])
        url_doc = documents[src_doc_idx]
        url_candidate = documents[trg_doc_idx]
        normdist = "0.0"

        if len(url_candidate) != 0 and len(url_doc) != 0:
            dist = Levenshtein.distance(url_doc, url_candidate)
            normdist = dist / max(len(url_doc), len(url_candidate))

        print("\t".join(fields) + "\t" + str(normdist))


if __name__ == '__main__':
    main()
