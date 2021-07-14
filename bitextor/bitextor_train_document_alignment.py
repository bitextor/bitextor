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

#
# 1. File .ridx is read and, for each line, the first candidate is chosen; the pair of indexes is stored in a
# dictionary. It is possible to provide two ridx files and combine them
# 2. The name of the aligned files are provided together with the clean text in base64 following this format:
#
# Output format:
#   file_lang1	file_lang2	cleantext_encoded_base64_lang1	cleantext_encoded_base64_lang2
#
import os
import sys
import argparse
import numpy as np
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, Activation
from tensorflow.keras.optimizers import SGD
from tensorflow.keras.callbacks import EarlyStopping, ModelCheckpoint

np.random.seed(1058)

oparser = argparse.ArgumentParser(
    description="usage: %prog [options]\nTool that processes a .ridx (reverse index) file (either from a file or from"
                "the standard input) and produces a list of aligned documents. If two ridx files are provided, a"
                "bidirectional alignment is performed between them.")
oparser.add_argument("-t", '--training', nargs='?',
                     help='File with extension .ridx (reverse index) for aligned documents', dest="trainingfile",
                     default=None)
oparser.add_argument("-d", '--development', nargs='?',
                     help='File with extension .ridx (reverse index) for aligned documents', dest="developmentfile",
                     default=None)
oparser.add_argument("-m", "--model",
                     help="If this option is defined, the rinal ridx file used for aligning the documents will be saved"
                          "in the path specified (when two ridx files are provided, the ridx obtained when merging both"
                          "files will be used)",
                     dest="model", default=None)
oparser.add_argument("-w", "--weights",
                     help="If this option is defined, the rinal ridx file used for aligning the documents will be saved"
                          "in the path specified (when two ridx files are provided, the ridx obtained when merging both"
                          "files will be used)",
                     dest="weights", default=None)
oparser.add_argument("-b", "--bestmodelof",
                     help="Number of times that the model is trained. Since the initialisation of the weights of the"
                          "regressor are random, it is convenient to train several models and keep that minimises the"
                          "error on the development set (default: 1)",
                     dest="nbest", default=1, type=int)

options = oparser.parse_args()

if options.trainingfile is None:
    reader = sys.stdin
else:
    reader = open(options.trainingfile, "r")

# Reading the .ridx file with the preliminary alignment
featurelist_train = []
featurelist_dev = []
labels_train = []
labels_dev = []

count = 0
for i in reader:
    count += 1
    featuresline = i.strip().split("\t")

    features = list(map(float, featuresline[2:-1]))
    label = float(featuresline[-1])
    featurelist_train.append(features)
    labels_train.append(label)

feats_array_train = np.array(featurelist_train)
labels_array_train = np.array(labels_train)

reader = open(options.developmentfile, "r")
for i in reader:
    featuresline = i.strip().split("\t")

    features = list(map(float, featuresline[2:-1]))
    label = float(featuresline[-1])
    featurelist_dev.append(features)
    labels_dev.append(label)
reader.close()

feats_array_dev = np.array(featurelist_dev)
labels_array_dev = np.array(labels_dev)

dimof_input = feats_array_train.shape[1]
dimof_output = len(set(labels_array_train.flat))

# labels_array_train = np_utils.to_categorical(labels_array_train, 2)
# labels_array_dev = np_utils.to_categorical(labels_array_dev, 2)


nmodel = 0
bestmodeliter = 0
bestmodel = None
minerr = 100000.0

for nmodel in range(0, options.nbest):
    model = Sequential()
    model.add(Dense(dimof_input * 2, init="uniform", activation='relu', input_shape=(dimof_input,)))
    model.add(Dropout(0.5, seed=1058))
    # model.add(Dense(dimof_input/2, init="uniform", activation='relu'))
    model.add(Dense(1, init='uniform'))
    model.add(Activation('sigmoid'))

    earlyStopping = EarlyStopping(patience=10, verbose=1, mode='auto')
    # checkpoint=ModelCheckpoint(options.weights, monitor='val_loss', verbose=0, save_best_only=True, mode='auto')
    checkpoint = ModelCheckpoint(options.weights, monitor='val_loss', verbose=0, save_best_only=True,
                                 save_weights_only=True, mode='auto', period=1)

    sgd = SGD(lr=0.5, decay=1e-4, momentum=0.1, nesterov=True)
    model.compile(loss='binary_crossentropy', optimizer=sgd)

    trainingout = model.fit(feats_array_train, labels_array_train, batch_size=128, nb_epoch=10000, verbose=1,
                            callbacks=[earlyStopping, checkpoint], validation_data=(feats_array_dev, labels_array_dev),
                            shuffle=True, class_weight=None, sample_weight=None)
    err = trainingout.history['val_loss'][-12]
    if err < minerr:
        bestmodeliter = nmodel
        minerr = err
        bestmodel = model
        bestmodel.load_weights(options.weights)
        os.remove(options.weights)

json_string = bestmodel.to_json()
modelf = open(options.model, "w")
modelf.write(json_string)
modelf.close()
bestmodel.save_weights(options.weights, overwrite=True)
