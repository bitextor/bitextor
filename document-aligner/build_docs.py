#!/usr/bin/env python


import argparse
from collections import defaultdict
import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../utils")
from common import open_xz_or_gzip_or_plain


def print_docs(docs_dict):
    not_found = []

    for k in docs_dict:
        en_url, fr_url = k

        if 'en_text' not in docs_dict[k]:
            not_found.append("not found(en): {0}".format(en_url))
            continue

        if 'fr_text' not in docs_dict[k]:
            not_found.append("not found(fr): {0}".format(fr_url))
            continue

        en_text, fr_text = docs_dict[k]['en_text'], docs_dict[k]['fr_text']
        print("{0}\t{1}\t{2}\t{3}".format(en_url, fr_url, en_text, fr_text))

    # if len(not_found):
    #     sys.stderr.write("Number of documents without matches: {0}\n".format(len(not_found)))
    #     for n in not_found:
    #         sys.stderr.write("{0}\n".format(n))


def get_text(docs_dict, map_e2f, map_f2e, url_filepath, text_filepath):
    with open_xz_or_gzip_or_plain(text_filepath) as f_text, open_xz_or_gzip_or_plain(url_filepath) as f_url:
        for line in f_text:
            text = line.strip()
            url = next(f_url, None).strip()

            if url in map_e2f:
                key = (url, map_e2f[url])
                docs_dict[key]['en_text'] = text
            elif url in map_f2e:
                key = (map_f2e[url], url)
                docs_dict[key]['fr_text'] = text


def load_docs(matches_filepath, url1_filepath, text1_filepath, url2_filepath, text2_filepath, threshold):
    map_e2f = {}
    map_f2e = {}

    docs_dict = defaultdict(dict)

    with open(matches_filepath, 'r') as f_mapping:
        for line in f_mapping:
            score, e, f = line.strip().split('\t')
            if float(score) < float(threshold):
                continue

            map_e2f[e] = f
            map_f2e[f] = e

    get_text(docs_dict, map_e2f, map_f2e, url1_filepath, text1_filepath)
    get_text(docs_dict, map_e2f, map_f2e, url2_filepath, text2_filepath)

    return docs_dict


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--matches', help='path to the file with matched documents', required=True)
    parser.add_argument('--url1', help='path to the file with the list of URLs crawled for LANG1', required=True)
    parser.add_argument('--plaintext1', help='path to the file with the plain text of the documents crawled for LANG1',
                        required=True)
    parser.add_argument('--url2', help='path to the file with the list of URLs crawled for LANG2', required=True)
    parser.add_argument('--plaintext2', help='path to the file with the plain text of the documents crawled for LANG2',
                        required=True)
    parser.add_argument('--threshold', help='documents with lower TF-IDF score will be skipped', default=0.1,
                        type=float, required=False)

    args = parser.parse_args()

    docs = load_docs(args.matches, args.url1, args.plaintext1, args.url2, args.plaintext2, args.threshold)
    print_docs(docs)
