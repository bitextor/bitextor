#!/usr/bin/env python3

import argparse
import sys
import os
import os.path
from collections import defaultdict

import numpy as np

from scorer import CosineDistanceScorer, WordExtractor, _ngram_helper

def match(score_matrix_csr, threshold):
    score_matrix_coo = score_matrix_csr.tocoo()
    matches_list = []
    visited_cols = set()
    visited_rows = set()

    smaller_dim = min(score_matrix_coo.shape)
    sorted_indices = np.argsort(
        score_matrix_coo.data, axis=None, kind='quicksort')

    for sorted_idx in sorted_indices[::-1]:
        curr_row = score_matrix_coo.row[sorted_idx]
        if curr_row in visited_rows:
            continue

        curr_col = score_matrix_coo.col[sorted_idx]
        if curr_col in visited_cols:
            continue

        if score_matrix_csr[int(curr_row), int(curr_col)] < threshold:
            break

        matches_list.append((curr_row, curr_col))
        visited_cols.add(curr_col)
        visited_rows.add(curr_row)

        if len(matches_list) >= smaller_dim:
            break

    match_costs_list = [score_matrix_csr[int(r), int(c)] for r, c in matches_list]

    return match_costs_list, matches_list


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--lang2', help='path to the extracted text', required=True)
    parser.add_argument(
        '--lang1', help='path to the translated foreign text', required=True)
    parser.add_argument('--min_count', type=int, default=2)
    parser.add_argument('--ngram_size', type=int, default=2)
    parser.add_argument('--tfidfsmooth', type=int, default=14)
    parser.add_argument('--output_matches', help='output file', required=True)
    parser.add_argument('--threshold', type=float, default=0.1)
    parser.add_argument('--batch_size', type=int, default=10000)
    parser.add_argument('--word_tokeniser', help='Word tokeniser executable path', required=True)

    args = parser.parse_args()

    #sys.stderr.write("threshold: {0}\n".format(args.threshold))
    #sys.stderr.write("batch_size: {0}\n".format(args.batch_size))

    if os.stat(args.lang1).st_size == 0 or os.stat(args.lang2).st_size == 0:
        sys.stderr.write(
            "WARNING: No document alignments feasible: " + str(os.stat("file").st_size) + " documents in foreign language and " + str(
                os.stat("file").st_size) + " documents in source language.\n")
        open(args.output_matches, 'a').close()
    elif (args.lang1[-3:] == ".xz" and os.stat(args.lang1).st_size == 32) or (args.lang2[-3:] == ".xz" and os.stat(args.lang2).st_size == 32):
        sys.stderr.write(
            "WARNING: No document alignments feasible: " + str(os.stat(args.lang1).st_size) + " documents in foreign language and " + str(
                os.stat(args.lang2).st_size) + " documents in source language.\n")
        open(args.output_matches, 'a').close()

    else:

        word_extractor = WordExtractor(
            n=args.ngram_size, ignore_set=None, word_tokeniser_cmd=args.word_tokeniser)
        scorer = CosineDistanceScorer(extraction_mapper=word_extractor,
                                      min_count=args.min_count,
                                      metric='cosine',
                                      smooth=args.tfidfsmooth,
                                      threshold=args.threshold,
                                      batch_size=args.batch_size)

        urls, m_csr = scorer.score(args.lang2, args.lang1)
        #sys.stderr.write(str(m_csr)+"\n")
        if m_csr is None:
            sys.stderr.write("WARNING: Documents do not contain any useful information to be used in alignment.\n")
            open(args.output_matches, 'a').close()
        else:
            match_costs, matches = match(m_csr, threshold=args.threshold)

            with open(args.output_matches, 'w') as f:
                for idx, match in enumerate(matches):
                    turl = urls[0][matches[idx][0]]
                    surl = urls[1][matches[idx][1]]
                    f.write("{0:.5f}\t{1}\t{2}\n".format(match_costs[idx], surl, turl))
