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
import re
import base64

from bitextor.utils.common import open_xz_or_gzip_or_plain


def extract_images(f, docs, offset=1):
    with open_xz_or_gzip_or_plain(f) as fd:
        for html_base64enc in fd:
            # To compute the edit distance at the level of characters, HTML tags must be encoded as characters and
            # not strings:
            html_content = base64.b64decode(html_base64enc.strip()).decode("utf-8", errors="ignore")
            links = re.findall('''<img [^>]*src\s*=\s*['"]\s*([^'"]+)['"]''', html_content, re.S)
            docs[offset] = set(list(links)) # Store imgs just once

            offset += 1

    return offset


def main():
    oparser = argparse.ArgumentParser(
        description="Script that rescores the aligned-document candidates provided by script bitextor-idx2ridx by using "
                    "the Levenshtein edit distance of the structure of the files.")
    oparser.add_argument('ridx', metavar='RIDX', nargs='?', default=None,
                         help='File with extension .ridx (reverse index) from bitextor-idx2ridx (if not provided, '
                         'the script will read from the standard input)')
    oparser.add_argument("--html1", help="File produced during pre-processing containing all HTML files in a WARC file",
                         dest="html1", required=True)
    oparser.add_argument("--html2", help="File produced during pre-processing containing all HTML files in a WARC file",
                         dest="html2", required=True)
    options = oparser.parse_args()

    if options.ridx is None:
        reader = sys.stdin
    else:
        reader = open(options.ridx, "r")

    documents = {"l1": {}, "l2": {}}

    extract_images(options.html1, documents["l1"])
    extract_images(options.html2, documents["l2"])

    header = next(reader).strip().split("\t")
    src_doc_idx_idx = header.index("src_index")
    trg_doc_idx_idx = header.index("trg_index")

    # Print output header
    print("\t".join(header) + "\timages_overlap_score")

    for i in reader:
        fields = i.strip().split("\t")
        src_doc_idx = int(fields[src_doc_idx_idx])
        trg_doc_idx = int(fields[trg_doc_idx_idx])
        urls_doc = documents["l1"][src_doc_idx]
        urls_candidate = documents["l2"][trg_doc_idx]
        bag_of_urls_overlap = 0

        if len(urls_doc.union(urls_candidate)) > 0:
            bag_of_urls_overlap = len(urls_doc.intersection(urls_candidate)) / float(len(urls_doc.union(urls_candidate)))

        print("\t".join(fields) + "\t" + str(bag_of_urls_overlap))


if __name__ == '__main__':
    main()
