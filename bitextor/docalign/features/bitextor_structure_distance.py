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
import base64
import math
import html.parser
import argparse

import Levenshtein

from bitextor.utils.common import open_xz_or_gzip_or_plain


class Parser(html.parser.HTMLParser):

    def error(self, message):
        pass

    def __init__(self):
        html.parser.HTMLParser.__init__(self)
        self.script = 0
        self.output = []

    def handle_starttag(self, tag, attrs):
        if tag == "script" or tag == "noscript":
            self.script = 1
        else:
            self.output.append("_" + tag + "_")

    def handle_data(self, data):
        if self.script == 0:
            if data != "":
                nwords = len(data.split())
                if nwords > 0:
                    # Replacing every word in text by a "_" symbol
                    self.output.append("_" * int(math.log(nwords, 2)))

    def handle_endtag(self, tag):
        if tag == "script" or tag == "noscript":
            self.script = 0
        else:
            self.output.append("_" + tag + "_")


# print("pathname", pathname)

def extract_structure_representations(f, docs, fileid):
    with open_xz_or_gzip_or_plain(f) as fd:
        dic = {}
        charidx = 32
        dic[''] = '_'

        for html_base64enc in fd:
            p = Parser()
            try:
                e = base64.b64decode(html_base64enc.strip()).decode("utf8", errors="ignore")
                if e != "":
                    p.feed(e)
                    raspa = "".join(p.output)
                    taglist = raspa.split('_')
                    if len(taglist) > 1 \
                            and taglist[1][-2:] == "ml" \
                            and all(ord(char) < 128 for char in raspa): # Check that all characters are ASCII
                        # Delete entries without *ml in the first  tag to avoid things different than HTML or XML
                        # as JPGS or PDF, for example
                        translated_taglist = []
                        for tag in taglist:
                            if tag not in dic:
                                # Adding new tags in the raspa and the character with which they will be replaced to the
                                # dictionary. To compute the edit distance at the level of characters, HTML tags must be
                                # encoded as characters and not strings:
                                dic[tag] = chr(charidx)
                                charidx += 1
                                if charidx == 95:
                                    charidx += 1

                            translated_taglist.append(dic[tag])
                        docs[fileid] = "".join(translated_taglist)
                    else:
                        docs[fileid] = " "
            except:
                docs[fileid] = " "
            finally:
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
    offset = extract_structure_representations(options.html1, documents, offset)
    offset = extract_structure_representations(options.html2, documents, offset)

    header = next(reader).strip().split("\t")
    src_doc_idx_idx = header.index("src_index")
    trg_doc_idx_idx = header.index("trg_index")

    # Print output header
    print("\t".join(header) + "\tstructure_distance")

    for i in reader:
        fields = i.strip().split("\t")
        src_doc_idx = int(fields[src_doc_idx_idx])
        trg_doc_idx = int(fields[trg_doc_idx_idx])
        len_s = len(documents[src_doc_idx])
        len_t = len(documents[trg_doc_idx])
        dist = Levenshtein.distance(documents[src_doc_idx], documents[trg_doc_idx])
        port = 1.0 - dist / max(len_s, len_t)

        print("\t".join(fields) + "\t" + str(port))


if __name__ == '__main__':
    main()
