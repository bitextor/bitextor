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


def extract_urls(html_file, url_file, docs, fileid):
    with open_xz_or_gzip_or_plain(html_file) as hd:
        with open_xz_or_gzip_or_plain(url_file) as ud:
            for url in ud:
                html_content = base64.b64decode(next(hd, None)).decode("utf-8", errors="ignore")
                links = re.findall('''href\s*=\s*['"]\s*([^'"]+)['"]''', html_content, re.S)
                docs[fileid] = [url, set(list(links))]
                fileid += 1
    return fileid


def main():
    oparser = argparse.ArgumentParser(
        description="Script that rescores the aligned-document candidates provided by script bitextor-idx2ridx by using "
                    "the Levenshtein edit distance of the structure of the files.")
    oparser.add_argument('ridx', metavar='RIDX', nargs='?', default=None,
                         help='File with extension .ridx (reverse index) from bitextor-idx2ridx (if not provided, '
                         'the script will read from the standard input)')
    oparser.add_argument("--html1", help="File produced during pre-processing containing all HTML files in a WARC file for SL",
                         dest="html1", required=True)
    oparser.add_argument("--html2", help="File produced during pre-processing containing all HTML files in a WARC file for TL",
                         dest="html2", required=True)
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
    offset = extract_urls(options.html1, options.url1, documents, offset)
    offset = extract_urls(options.html2, options.url2, documents, offset)

    header = next(reader).strip().split("\t")
    src_doc_idx_idx = header.index("src_doc_idx")
    trg_doc_idx_idx = header.index("trg_doc_idx")

    # Print output header
    print("\t".join(header) + "\tsrc_doc_linked_by_trg_doc")

    for i in reader:
        fields = i.strip().split("\t")
        src_doc_idx = int(fields[src_doc_idx_idx])
        trg_doc_idx = int(fields[trg_doc_idx_idx])
        url_doc = documents[src_doc_idx][0]
        urls_candidate = documents[trg_doc_idx][1]
        candidate = "0.0"

        if url_doc in urls_candidate:
            candidate = "1.0"

        print("\t".join(fields) + "\t" + str(candidate))


if __name__ == '__main__':
    main()
