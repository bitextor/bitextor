#!__ENV__ __PYTHON__

#
# 1. Reads LETT file and stores it as a dict object in python
# 2. Reads RIDX file
# 3. Uses a regressor to combine all the scores (features) assigned to each pair of documents and obtain a similarity metric that combines all of them
# 4. Producing a new RIDX file that contains a single score for each pair of documents
#
# Expected input format:
# num_doc_lang1    [num_doc_lang2:score1:score2:score3...]+
#
#

import sys
import argparse
from operator import itemgetter
from keras.models import model_from_json
import numpy as np
from keras.utils import np_utils
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

oparser = argparse.ArgumentParser(description="Script that rescores the aligned-document candidates provided by script bitextor-idx2ridx by using the Levenshtein edit distance of the structure of the files.")
oparser.add_argument('ridx', metavar='RIDX', nargs='?', help='File with extension .ridx (reverse index) from bitextor-idx2ridx (if not provided, the script will read from the standard input)', default=None)
oparser.add_argument("-w", "--weights", help="File where the weights of the model to be used for document alignment are stored", dest="weights", required=True)
oparser.add_argument("-m", "--model", help="File where the architecture of the model to be used for document alignment is stored", dest="model", required=True)
oparser.add_argument("-t", "--threshold", help="Threshold for the score assigned to each candidate", dest="threshold", type=float, default=0.0)
options = oparser.parse_args()

#Loading classifier from file
model = model_from_json(open(options.model).read())
model.load_weights(options.weights)

if options.ridx == None:
  reader = sys.stdin
else:
  reader = open(options.ridx,"r")

for i in reader:
  fields = i.strip().split("\t")
  fileid1=int(fields[0])

  newscores = {}
  for candidate in fields[1:]:
    fileid2=int(candidate.split(":")[0])
    features=list(map(float, candidate.split(":")[1:]))
    newscores[fileid2]=model.predict_proba(np.array([features]), batch_size=1, verbose=0)[0]
  new_rank=sorted(list(newscores.items()), key=itemgetter(1), reverse=True)
  sys.stdout.write(str(fileid1))
  for sorteditem in new_rank:
    fileid2 = sorteditem[0]
    score = sorteditem[1][0]
    if score >= options.threshold:
      sys.stdout.write("\t")
      sys.stdout.write(str(fileid2))
      sys.stdout.write(":")
      sys.stdout.write(str(score))
  print()
