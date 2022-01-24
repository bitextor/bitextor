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


def extract_images(f, docs, fileid):
    with open_xz_or_gzip_or_plain(f) as fd:
        for html_base64enc in fd:
            # To compute the edit distance at the level of characters, HTML tags must be encoded as characters and
            # not strings:
            links = re.findall('''<img [^>]*src\s*=\s*['"]\s*([^'"]+)['"]''',
                               base64.b64decode(html_base64enc.strip()).decode("utf-8", errors="ignore"), re.S)
            docs[fileid] = set(list(links))
            fileid += 1
    return fileid


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

    documents = {}
    offset = 1
    offset = extract_images(options.html1, documents, offset)
    offset = extract_images(options.html2, documents, offset)

    header = next(reader).strip().split("\t")
    doc_idx_idx = header.index("doc_idx")
    best_matches_idx_and_score_idx = header.index("best_matches_idx_and_score")

    # Print output header
    # TODO are we sure we want to carry on appending values to the same column? We might add padding values with score 0 for a better process...
    print("doc_idx\tbest_matches_idx_and_score_and_image_score")

    for i in reader:
        fields = i.strip().split("\t")
        doc_idx = fields[doc_idx_idx]
        best_matches = fields[best_matches_idx_and_score_idx]
    
        # The document must have at least one candidate
        if best_matches != "":
            urls_doc = documents[int(doc_idx)]
            matches = best_matches.split("_")
            candidates = []

            sys.stdout.write(doc_idx + "\t")

            for match in matches:
                candidate = match
                candidateid = int(match.split(":")[0])
                urls_candidate = documents[candidateid]

                if len(urls_doc.union(urls_candidate)) > 0:
                    bagofurlsoverlap = len(urls_doc.intersection(urls_candidate)) / float(len(urls_doc.union(urls_candidate)))
                else:
                    bagofurlsoverlap = 0

                candidate += ":" + str(bagofurlsoverlap)

                candidates.append(candidate)

            sys.stdout.write("_".join(candidate) + "\n")

if __name__ == '__main__':
    main()
