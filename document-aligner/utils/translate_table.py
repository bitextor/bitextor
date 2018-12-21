#!/usr/bin/env python3

import argparse
import sys

from common import open_gzip_or_plain


def build_translations(translation_file):
    table = {}
    with open_gzip_or_plain(translation_file) as f:
        for line in f:
            line_split = line.strip().split('\t', 1)
            if len(line_split) != 2:
                sys.stderr.write("Translation table error on line: {0}\n".format(str(line_split)))
                continue

            table[line_split[0]] = line_split[1]

    return table


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Translate sentences from stdin using a translation table with already translated sentences.')
    parser.add_argument('--translations', dest='translations',
                        help='translation-table file', required=True)
    args = parser.parse_args()

    translations = build_translations(args.translations)
    not_found_num = 0

    for line in sys.stdin:
        line = line.strip()
        if line in translations:
            # If found, use the translation
            print(translations[line])
        else:
            # If not found, write to stderr
            # sys.stderr.write("not_found: {0}\n".format(line))
            not_found_num += 1
            print(line)

    sys.stderr.write("Number of translations without matches: {0}\n".format(not_found_num))
