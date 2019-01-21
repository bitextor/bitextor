#!/usr/bin/env python


import argparse
from collections import defaultdict
import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../utils")
from common import open_xz_or_gzip_or_plain


def print_docs(docs):
    not_found = []

    for k in docs:
        en_url, fr_url = k

        if 'en_text' not in docs[k]:
            not_found.append("not found(en): {0}".format(en_url))
            continue

        if 'fr_text' not in docs[k]:
            not_found.append("not found(fr): {0}".format(fr_url))
            continue

        en_text, fr_text = docs[k]['en_text'], docs[k]['fr_text']
        print("{0}\t{1}\t{2}\t{3}\t".format(en_url, fr_url, en_text, fr_text))

    if len(not_found):
        sys.stderr.write("Number of documents without matches: {0}\n".format(len(not_found)))
        for n in not_found:
            sys.stderr.write("{0}\n".format(n))


def load_docs(matches_filepath, url_filepath, text_filepath, threshold):
    map_e2f = {}
    map_f2e = {}

    docs = defaultdict(dict)

    with open(matches_filepath, 'r') as f_mapping:
        for line in f_mapping:
            score, e, f = line.strip().split('\t')
            if float(score) < float(threshold):
                continue

            map_e2f[e] = f
            map_f2e[f] = e

    with open_xz_or_gzip_or_plain(text_filepath) as f_text, open_xz_or_gzip_or_plain(url_filepath) as f_url:
        for line in f_text:
            text=line.strip()
            url=next(f_url, None).strip()

            if url in map_e2f:
                key = (url, map_e2f[url])
                docs[key]['en_text'] = text

            elif url in map_f2e:
                key = (map_f2e[url], url)
                docs[key]['fr_text'] = text

    return docs


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--matches', help='path to the file with matched documents', required=True)
    parser.add_argument('--url', help='path to the file with the list of URLs crawled', required=True)
    parser.add_argument('--plaintext', help='path to the file with the plain text of the documents crawled', required=True)
    parser.add_argument('--threshold', help='documents with lower TF-IDF score will be skipped', default=0.1, type=float, required=False)

    args = parser.parse_args()

    docs = load_docs(args.matches, args.url, args.plaintext, args.threshold)
    print_docs(docs)
