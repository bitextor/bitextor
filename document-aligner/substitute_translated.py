#!/usr/bin/env python

import argparse
import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../utils")
from common import build_mappings, check_lengths


if __name__ == "__main__":

    oparser = argparse.ArgumentParser(description="")
    oparser.add_argument("--deduplicated", help="Deduplicated text",
                         dest="deduplicated", required=True)
    oparser.add_argument("--translated", help="Translated text",
                         dest="translated", required=True)

    options = oparser.parse_args()

    sys.stderr.write("Building substitution mappings...\n")
    check_lengths(options.deduplicated, options.translated)
    mapping = build_mappings(options.deduplicated, options.translated)
    sys.stderr.write("Done.\n")

    line_num = 0
    err_num = 0
    for line in sys.stdin:
        line_num += 1
        split_line = line.strip().split('\t', 1)

        if len(split_line) == 1:
            print("{0}\t{1}".format(split_line[0], ""))
            continue

        if len(split_line) != 2:
            sys.stderr.write("Substitution error on line {0}: {1}\n".format(line_num, str(split_line)))
            continue

        url, text = split_line
        if text not in mapping:
            err_num += 1
            # sys.stderr.write("Substitution not_found on line {0}: {1}\n".format(line_num, str(split_line)))
            print("{0}\t{1}".format(url, text))
        else:
            print("{0}\t{1}".format(url, mapping[text]))

    sys.stderr.write("Detected sentences: {0}\n".format(line_num - err_num))
    sys.stderr.write("Undetected sentences: {0}\n".format(err_num))

    if err_num != 0:
        sys.stderr.write("Error while substituting: {0} sentences were not found!\n".format(err_num))
