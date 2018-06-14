#!/usr/bin/python

import sys
import matplotlib.pyplot as plt
import numpy as np
from sklearn import linear_model, datasets
import pickle

train_model = sys.argv[1]
test_data   = sys.argv[2]
output_txt  = sys.argv[3]

logreg = pickle.load(open(train_model, 'rb'))

devX = np.loadtxt(test_data)
Z = logreg.decision_function(devX)
np.savetxt(output_txt, Z, "%s")
