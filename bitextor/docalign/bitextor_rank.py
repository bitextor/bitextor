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


def print_scores(model, src_doc_idx, candidate_list, features_list, threshold=0.0):
    predictions = model.predict_proba(np.array(features_list))

    scores = [
        (doc, prob1)
        for doc, (_, prob1) in zip(candidate_list, predictions)
        if prob1 >= threshold
    ]

    if len(scores) != 0:
        scores.sort(key=itemgetter(1), reverse=True)

        for doc, score in scores:
            print(f"{src_doc_idx}\t{doc}\t{score}")


def main():
    options = parse_args()

    model = joblib.load(options.model)

    features_desc = ["bow_overlap_score", "images_overlap_score", "structure_distance", "document_urls_distance",
                     "src_doc_linked_by_trg_doc", "urls_distance", "urls_overlap_score"]

    header = next(sys.stdin).strip().split("\t")
    src_doc_idx_idx = header.index("src_doc_idx")
    trg_doc_idx_idx = header.index("trg_doc_idx")

    # Print output header
    print("src_doc_idx\ttrg_doc_idx\trank_score")

    last_src_doc_idx = -1
    candidate_list = []
    features_list = []

    for line in sys.stdin:
        fields = line.strip().split("\t")
        src_doc_idx = int(fields[src_doc_idx_idx])
        trg_doc_idx = int(fields[trg_doc_idx_idx])

        if last_src_doc_idx < 0:
            last_src_doc_idx = src_doc_idx
        if last_src_doc_idx != src_doc_idx:
            print_scores(model, last_src_doc_idx, candidate_list, features_list, options.threshold)

            candidate_list = []
            features_list = []

        features = [float(feature) for feature in [fields[header.index(feature_name)] for feature_name in features_desc]]

        candidate_list.append(trg_doc_idx)
        features_list.append(features)

        last_src_doc_idx = src_doc_idx

    # Print the last elements
    if len(candidate_list) != 0:
        print_scores(model, last_src_doc_idx, candidate_list, features_list, options.threshold)

        candidate_list = []
        features_list = []


if __name__ == "__main__":
    main()
