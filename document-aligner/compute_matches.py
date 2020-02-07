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

import argparse
import sys
import os.path

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
    parser.add_argument('--lang1', help='path to the tokenized translated foreign text, with one base64 document per line', required=True)
    parser.add_argument('--lang2', help='path to the tokenized lang2 text, with one base64 document per line', required=True)
    parser.add_argument('--min_count', type=int, default=2)
    parser.add_argument('--ngram_size', type=int, default=2)
    parser.add_argument('--tfidfsmooth', type=int, default=14)
    parser.add_argument('--output_matches', help='output file', required=True)
    parser.add_argument('--threshold', type=float, default=0.1)
    parser.add_argument('--batch_size', type=int, default=10000)
    parser.add_argument('-j', '--jobs', type=int, default=1, dest='jobs')
    args = parser.parse_args()

    if args.jobs <= 0:
        args.jobs = os.cpu_count() - args.jobs

    # sys.stderr.write("threshold: {0}\n".format(args.threshold))
    # sys.stderr.write("batch_size: {0}\n".format(args.batch_size))

    if os.stat(args.lang1).st_size == 0 or os.stat(args.lang2).st_size == 0:
        sys.stderr.write(f'WARNING: No document alignments feasible: {args.lang1} or {args.lang2} is empty')
        open(args.output_matches, 'a').close()
    elif (args.lang1[-3:] == ".xz" and os.stat(args.lang1).st_size == 32) or (args.lang2[-3:] == ".xz" and os.stat(args.lang2).st_size == 32):
        sys.stderr.write(f'WARNING: No document alignments feasible: {args.lang1} or {args.lang2} is empty')
        open(args.output_matches, 'a').close()
    elif (args.lang1[-3:] == ".gz" and os.stat(args.lang1).st_size == 26) or (args.lang2[-3:] == ".gz" and os.stat(args.lang2).st_size == 26):
        sys.stderr.write(f'WARNING: No document alignments feasible: {args.lang1} or {args.lang2} is empty')
        open(args.output_matches, 'a').close()

    else:
        word_extractor = WordExtractor(n=args.ngram_size, ignore_set=None)
        scorer = CosineDistanceScorer(extraction_mapper=word_extractor,
                                      min_count=args.min_count,
                                      smooth=args.tfidfsmooth,
                                      threshold=args.threshold,
                                      batch_size=args.batch_size,
                                      jobs=args.jobs)

        m_csr = scorer.score(args.lang2, args.lang1)
        # sys.stderr.write(str(m_csr)+"\n")
        if m_csr is None:
            sys.stderr.write("WARNING: Documents do not contain any useful information to be used in alignment.\n")
            open(args.output_matches, 'a').close()
        else:
            match_costs, matches = match(m_csr, threshold=args.threshold)

            with open(args.output_matches, 'w') as f:
                for idx, match in enumerate(matches):
                    turl = matches[idx][0]+1
                    surl = matches[idx][1]+1
                    f.write("{0:.5f}\t{1}\t{2}\n".format(match_costs[idx], surl, turl))
