#!/usr/bin/env python3

import joblib
import argparse
import sys
import numpy as np
from operator import itemgetter


def parse_args():
    oparser = argparse.ArgumentParser(prog="bitextor_rank.py",
        description="Script that from stdin computed features of possible document pairings "
        "and writes to stdout ranked pairings ordered by their score \n\n"
        "Input format is: <src_id> \\t <trg_id>:<f1>:<...>:<f7> [\\t <trg_id>:<f1>:<...>:<f7> ...]\n"
        "Output format is: <src_id> \\t <trg_id>:<score> [ \\t <trg_id>:<score> ...]",
        formatter_class=argparse.RawDescriptionHelpFormatter)
    oparser.add_argument("-t", "--threshold", type=float, default=0.0, help="Ignore pairing below this threshold")
    oparser.add_argument("-m", "--model", dest="model", required=True, help="sklearn model used for ranking")
    return oparser.parse_args()


def main():
    options = parse_args()

    model = joblib.load(options.model)

    features_desc = ["bag-of-words", "imgoverlap", "structedistance", "urldistance", "mutuallylinked", "urlscomparison", "urlsoverlap"]

    for line in sys.stdin:
        fields = line.strip().split("\t")


        if len(fields) < 2:
            continue

        doc1 = int(fields[0])

        candidate_list = []
        features_list = []

        for candidate in fields[1:]:
            candidate_fields = candidate.split(":")
            candidate_list.append(int(candidate_fields[0]))
            features_list.append([float(feat) for feat in candidate_fields[1:]])

        predictions = model.predict_proba(np.array(features_list))

        scores = [
            (doc, prob1)
            for doc, (_, prob1) in zip(candidate_list, predictions)
            if prob1 >= options.threshold
        ]

        if len(scores) == 0:
            continue

        scores.sort(key=itemgetter(1), reverse=True)

        print(f"{doc1}", end="")
        for doc, score in scores:
            print(f"\t{doc}:{score}", end="")
        print()


if __name__ == "__main__":
    main()
