#!/usr/bin/env python3

import logging
import gzip
import argparse
import logging
from os import read
import joblib
import numpy as np

from sklearn.ensemble import ExtraTreesClassifier, RandomForestClassifier
from sklearn.model_selection import GridSearchCV
from sklearn.metrics import classification_report
from sklearn.svm import SVC
from operator import itemgetter

def parse_args():
  oparser = argparse.ArgumentParser("Script used to train feature based document aligner classifier")
  oparser.add_argument("--train", nargs="+", required=True, help="Train corpus. Format is \"<doc_id1> \\t <doc_id1> \\t <feature1> \\t ... <feature7> \\t <label>\"")
  oparser.add_argument("--test", nargs="*", help="Test corpus")
  oparser.add_argument("--classifier-type", choices=["ExtraTrees", "RandomForest", "SVM"], default="ExtraTrees", help="Classifier type", dest="type")
  oparser.add_argument("-m", "--model", dest="model", default="docalign.model", help="Output file where model will be stored")
  oparser.add_argument("-v", "--verbose", action="store_true", default=False, help="Print debug information")
  oparser.add_argument("-q", "--quiet", action="store_true", default=False, help="Print only errors")

  return oparser.parse_args()


def read_corpus(file_list):
  X_list = []
  Y_list = []
  for input_file in file_list:
    with gzip.open(input_file, "rt") as fd:
      for line in fd:
        fields = line.strip().split("\t")
        features = [float(x) for x in fields[2:-1]]
        X_list.append(features)
        Y_list.append(int(fields[-1]))

  return np.array(X_list), np.array(Y_list)


def main():
  options = parse_args()

  level = logging.INFO
  if options.verbose:
    level = logging.DEBUG
  elif options.quiet:
    level = logging.ERROR

  logging.basicConfig(level=level, format="%(levelname)s: %(message)s")

  features_desc = ["bag-of-words", "imgoverlap", "structedistance", "urldistance", "mutuallylinked", "urlscomparison", "urlsoverlap"]

  features_array, labels_array = read_corpus(options.train)
  logging.info(f"X_test shape: {features_array.shape}")
  logging.info(f"Y_test shape: {labels_array.shape}")

  if options.test:
    test_features_array, test_labels_array = read_corpus(options.test)
    logging.info(f"X_test shape: {test_features_array.shape}")
    logging.info(f"Y_test shape: {test_labels_array.shape}")

  parameters = {}
  if options.type in ("ExtraTrees", "RandomForest"):
      parameters = {
          "criterion": ("gini", "entropy"),
          "n_estimators": [200, 400, 600, 800]
      }
      if options.type == "ExtraTrees":
        clf = ExtraTreesClassifier(
            bootstrap=True,
            criterion="gini",
            n_estimators=600,
            n_jobs=1,
            random_state=0,
        )
      elif options.type == "RandomForest":
        clf = RandomForestClassifier(
            bootstrap=True,
            criterion="gini",
            n_estimators=600,
            n_jobs=1,
            random_state=0
        )
      # clf = GridSearchCV(clf, parameters)

  elif options.type == "SVM":
    parameters = {
      "C": [1, 10, 100],
      "gamma": [0.01, 0.1, 'scale', 'auto']
    }

    # clf = GridSearchCV(SVC(probability=True), parameters)
    clf = SVC(probability=True, C=1, gamme=0.0)


  clf.fit(features_array, labels_array)


  if isinstance(clf, GridSearchCV):

      logging.info("Grid scores on development set:")
      means = clf.cv_results_['mean_test_score']
      stds = clf.cv_results_['std_test_score']
      for mean, std, params in zip(means, stds, clf.cv_results_['params']):
          logging.info(f"{mean:.3f} (+/-{std*2:.3f}) for {params}")

      logging.info("Best classifier parameters found:")
      for k,v in clf.best_params_.items():
          logging.info(f"\t{k}: {v}")

      clf = clf.best_estimator_

  if options.type in ("ExtraTrees", "RandomForest"):
    feat_dict = dict(zip(features_desc, clf.feature_importances_))
    sorted_feat = sorted(feat_dict.items(), key=itemgetter(1), reverse=True)
    logging.info("Important features:")
    for (name, value) in sorted_feat:
      logging.info(f"\t{value:.4f}: {name}")

  joblib.dump(clf, options.model)

  if options.test:
    predictions = clf.predict(test_features_array)
    logging.info("Detailed classification report: ")
    report = classification_report(test_labels_array, predictions)
    for line in report.split("\n"):
        logging.info(f"{line}")

if __name__ == "__main__":
  main()
