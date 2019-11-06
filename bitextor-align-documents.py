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

# 1. Two RIDX files and and LETT file are taken as the input by this script. LETT contains information about all the
# files in the website. Each RIDX files contains, for each document in a language, the list of most promising files
# in another language to be parallel and a confidence score.

# 2. RIDX files are read and most promising document pairs are aligned.

# 3. The output of the script is a tab-separated file where each line contains the URLs of both files and their
# content (in plain text) encoded in base64
# 
# Output format:
#   file_lang1	file_lang2	plaintext_encoded_base64_lang1	plaintext_encoded_base64_lang2
#

import os
import sys
import argparse
from operator import itemgetter
import lzma

oparser = argparse.ArgumentParser(description="usage: %prog [options]\nTool that processes a .ridx (reverse index) "
                                              "file (either from a file or from the standard input) and produces a "
                                              "list of aligned documents. If two ridx files are provided, "
                                              "a bidirectional alignment is performed between them.")
oparser.add_argument('ridx1', metavar='RIDX', nargs='?', help='File with extension .ridx (reverse index) for aligned '
                                                              'documents from lang1 to lang2', default=None)
oparser.add_argument('ridx2', metavar='RIDX', nargs='?', help='File with extension .ridx (reverse index) for aligned '
                                                              'documents from lang2 to lang1', default=None)
oparser.add_argument('--lines1', dest='ndoc1', help='Number of documents in lang1', required=True, type=int)
oparser.add_argument('--lines2', dest='ndoc2', help='Number of documents in lang2', required=True, type=int)
oparser.add_argument("-n", "--num_candidates", help="Amount of alignment candidates taken into account for every file "
                                                    "when performing bidirectional document alignment. This parameter "
                                                    "is set by default to 1, which means that only documents being "
                                                    "mutually the best alignment option will be aligned. Note that "
                                                    "this option is only used when two ridx files are provided",
                     type=int, dest="candidate_num", default=1)
oparser.add_argument("-r", "--ridx", help="If this option is defined, the final ridx file used for aligning the "
                                          "documents will be saved in the path specified (when two ridx files are "
                                          "provided, the ridx obtained when merging both files will be used)",
                     dest="oridx", type=argparse.FileType('w'), default=None)
oparser.add_argument("-i", "--iterations", help="Number of iterations to keep looking for paired documents (only "
                                                "works without the non-symmetric option, if both options, then uses 1 "
                                                "iteration). Can also do undetermined iterations until all possible "
                                                "documents are paired. To do that, just write 'converge' as number of "
                                                "iterations", dest="iterations", default="1")
oparser.add_argument("-s", "--nonsymmetric", help="Write document alignments even if they are not backwards aligned ("
                                                  "option incompatible with 'iterations' option)",
                     dest="nonsymmetric", action="store_true")

options = oparser.parse_args()
reader = None
reader1 = None
reader2 = None
combine = False
if options.ridx2 is None:
    if options.ridx1 is None:
        reader = sys.stdin
    else:
        if options.ridx1[-3:] == ".xz":
            reader = lzma.open(options.ridx1, "rt")
        else:
            reader = open(options.ridx1, "r")
else:
    combine = True
    if options.ridx1[-3:] == ".xz":
        reader1 = lzma.open(options.ridx1, "rt")
    else:
        reader1 = open(options.ridx1, "r")
    if options.ridx2[-3:] == ".xz":
        reader2 = lzma.open(options.ridx2, "rt")
    else:
        reader2 = open(options.ridx2, "r")

indices = {}
indicesProb = {}
documents = set(range(1, options.ndoc1 + options.ndoc2 + 1))
documentsFile2 = set()
file2_start_counter = options.ndoc1

if not combine:
    # Reading the .ridx file with the preliminary alignment
    for i in reader:
        fields = i.split("\t")
        if len(fields) >= 2:
            if options.oridx is not None:
                options.oridx.write(i)
            try:
                indices[int(fields[0])] = int(fields[1].strip().split(":")[0])
            except:
                pass
    reader.close()
    if options.oridx is not None:
        options.oridx.close()
else:
    iterations = ""
    if options.nonsymmetric:
        iterations = "1"
    else:
        iterations = options.iterations

    converge = False
    if iterations == "converge":
        iterations = "1"
        if not options.nonsymmetric:
            converge = True

    iterationList = list(range(0, int(iterations)))
    pairedDocs = set()
    # In each iteration we read the whole file of document relationships and candidates,
    # but we ignore already paired documents
    for iteration in iterationList:
        reader2.seek(0)
        reader1.seek(0)
        if converge:
            # We create an infinite loop adding 1 item to the iteration list
            # if we want to stop when the algorithm converges
            iterationList.append(iteration + 1)

        # We store both directions of document relationships for printing purposes
        best_ridx2_inv = {}
        best_ridx2 = {}
        candidateDocuments = []

        # Reading the .ridx file with the preliminary alignment in one of the directions
        for line_ridx2 in reader2:
            fields = line_ridx2.strip().split("\t")
            if len(fields) >= 2:
                num_candidates = min(len(fields) - 1, options.candidate_num)
                candidateIterations = list(range(1, num_candidates + 1))
                for candidate_idx in candidateIterations:
                    field_n = fields[candidate_idx].split(":")
                    if len(field_n) == 2 and int(fields[0]) not in pairedDocs and int(field_n[0]) not in pairedDocs:
                        # Avoid pairing docs already paired in previous iterations of the algorithm,
                        # in case you specified the parameter
                        if int(field_n[0]) not in best_ridx2_inv:
                            best_ridx2_inv[int(field_n[0])] = {}
                            documentsFile2.add(int(fields[0]))
                        best_ridx2_inv[int(field_n[0])][int(fields[0])] = field_n[1]
                        if int(fields[0]) not in best_ridx2:
                            best_ridx2[int(fields[0])] = {}
                        best_ridx2[int(fields[0])][int(field_n[0])] = field_n[1]
                    elif int(field_n[0]) in pairedDocs and num_candidates < (len(fields) - 1):
                        # If the documents are already paired, we just ignored the read candidate
                        # and do another iteration
                        num_candidates = num_candidates + 1
                        candidateIterations.append(num_candidates - 1)

        # Reading the .ridx file with the preliminary alignment in the other direction
        # and combining this information with the previous one
        pairedDocsLine = set()
        for line_ridx1 in reader1:
            new_candidate_list = {}
            fields = line_ridx1.strip().split("\t")
            if len(fields) >= 2:
                num_candidates = min(len(fields) - 1, options.candidate_num)
                candidateIterations = list(range(1, num_candidates + 1))
                for candidate_idx in candidateIterations:
                    field_n = fields[candidate_idx].split(":")
                    if len(field_n) == 2 and int(field_n[0]) not in pairedDocs and int(fields[0]) not in pairedDocs:
                        # Same check for already paired documents in several iterations
                        documentsFile2.add(int(field_n[0]))
                        if int(fields[0]) in best_ridx2_inv and int(field_n[0]) in best_ridx2_inv[int(fields[0])]:
                            average = (float(best_ridx2_inv[int(fields[0])][int(field_n[0])]) + float(field_n[
                                                                                                          1])) / 2  #
                            # TODO: we should implement also the product of the score/probability as an option/parameter
                            new_candidate_list[field_n[0]] = average
                        if options.nonsymmetric:
                            if field_n[0] not in new_candidate_list:
                                new_candidate_list[field_n[0]] = float(field_n[1])
                    elif int(field_n[0]) in pairedDocs and num_candidates < (
                            len(fields) - 1):  # Same check to keep iterating and ignoring already paired documents
                        num_candidates = num_candidates + 1
                        candidateIterations.append(num_candidates - 1)

            if len(new_candidate_list) >= 1:
                candidateDocuments.append(len(new_candidate_list))
                sorted_candidates = sorted(iter(new_candidate_list.items()), key=itemgetter(1), reverse=True)
                if int(fields[0]) not in indicesProb or indicesProb[int(fields[0])] < int(sorted_candidates[0][1]):
                    # We store the final indices and their scores, in case of any symmetric relationship overlap
                    indices[int(fields[0])] = int(sorted_candidates[0][0])
                    indicesProb[int(fields[0])] = float(sorted_candidates[0][1])
                pairedDocsLine.add(int(fields[0]))
                pairedDocsLine.add(int(sorted_candidates[0][0]))
                if options.oridx is not None:
                    elements = ["{0}:{1}".format(candidate_tuple[0], candidate_tuple[1]) for candidate_tuple in
                                sorted_candidates]
                    options.oridx.write("{0}\t{1}\n".format(fields[0], "\t".join(elements)))

        pairedDocs.update(pairedDocsLine)

        if options.nonsymmetric:
            for document in sorted(list(documents - pairedDocs)):
                # We now print the relationships of the first read relationships file with unpaired documents
                if int(document) in best_ridx2 and len(best_ridx2[int(document)]) >= 1:
                    new_candidate_list = best_ridx2[int(document)]
                    candidateDocuments.append(len(new_candidate_list))
                    sorted_candidates = sorted(iter(new_candidate_list.items()), key=itemgetter(1), reverse=True)
                    if int(document) not in indicesProb or indicesProb[int(document)] < int(sorted_candidates[0][1]):
                        indices[int(document)] = int(sorted_candidates[0][0])
                        indicesProb[int(document)] = float(sorted_candidates[0][1])
                    pairedDocs.add(int(document))
                    pairedDocs.add(int(sorted_candidates[0][0]))
                    if options.oridx is not None:
                        elements = ["{0}:{1}".format(candidate_tuple[0], candidate_tuple[1]) for candidate_tuple in
                                    sorted_candidates]
                        for element in elements:
                            score = element.split(':')[1]
                            document2 = element.split(':')[0]
                            options.oridx.write("{0}\t{1}:{2}\n".format(document2, document, score))
        # End of the iterations if the result did not change from the previous iteration
        # (exit point of the endless loop)
        if len(candidateDocuments) == 0:
            break
    # Close files
    reader2.close()
    reader1.close()
    if options.oridx is not None:
        options.oridx.close()

for k in indices:
    if indices[k] in documents:
        if indices[k] in documentsFile2:
            # Write the output keeping the language documents always in the same order
            # (this is done because of the non-symmetric option that causes swaps in the last algorithm)
            print("{0}\t{1}".format(str(k), str(indices[k]-file2_start_counter)))
        else:
            print("{1}\t{0}".format(str(k-file2_start_counter), str(indices[k])))
        if not options.nonsymmetric:
            documents.remove(k)
