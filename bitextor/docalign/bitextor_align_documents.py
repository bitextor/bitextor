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

# 3. The output of the script is a tab-separated file where each line contains the URLs of both files
#
# Output format:
#   file_lang1	file_lang2	plaintext_encoded_base64_lang1	plaintext_encoded_base64_lang2
#

import sys
import argparse
from operator import itemgetter

from bitextor.utils.common import open_xz_or_gzip_or_plain, dummy_open

oparser = argparse.ArgumentParser(description="usage: %prog [options]\nTool that processes a .ridx (reverse index) "
                                              "file (either from a file or from the standard input) and produces a "
                                              "list of aligned documents. If two ridx files are provided, "
                                              "a bidirectional alignment is performed between them.")

oparser.add_argument('ridx1', metavar='RIDX', nargs='?', default=None,
                     help='File with extension .ridx (reverse index) for aligned '
                     'documents from lang1 to lang2')
oparser.add_argument('ridx2', metavar='RIDX', nargs='?', default=None,
                     help='File with extension .ridx (reverse index) for aligned '
                     'documents from lang2 to lang1')

oparser.add_argument('--lines1', dest='ndoc1', required=True, type=int, help='Number of documents in lang1')
oparser.add_argument('--lines2', dest='ndoc2', required=True, type=int, help='Number of documents in lang2')

oparser.add_argument("-n", "--num_candidates", type=int, dest="candidate_num", default=1,
                     help="Amount of alignment candidates taken into account for every file "
                     "when performing bidirectional document alignment. This parameter "
                     "is set by default to 1, which means that only documents being "
                     "mutually the best alignment option will be aligned. Note that "
                     "this option is only used when two ridx files are provided")

oparser.add_argument("-r", "--ridx", dest="oridx", default=None,
                    help="If this option is defined, the final ridx file used for aligning the "
                    "documents will be saved in the path specified (when two ridx files are "
                    "provided, the ridx obtained when merging both files will be used)")

oparser.add_argument("-i", "--iterations", dest="iterations", default="1",
                    help="Number of iterations to keep looking for paired documents (only "
                    "works without the non-symmetric option, if both options, then uses 1 "
                    "iteration). Can also do undetermined iterations until all possible "
                    "documents are paired. To do that, just write 'converge' as number of "
                    "iterations")

oparser.add_argument("-s", "--nonsymmetric", dest="nonsymmetric", action="store_true",
                    help="Write document alignments even if they are not backwards aligned ("
                    "option incompatible with 'iterations' option)")

oparser.add_argument("-t", "--threshold", type=float, default=0.0,
                    help="Discard document below a certain threshold. If two .ridx files "
                    "are provided, the threshold is computed by computing the average of "
                    "the individual thresholds")

options = oparser.parse_args()

indices = {}
indicesProb = {}
documents = set(range(1, options.ndoc1 + options.ndoc2 + 1))
documentsFile2 = set()
file2_start_counter = options.ndoc1


if options.ridx2 is None:
    # Reading the .ridx file with the preliminary alignment
    with open_xz_or_gzip_or_plain(options.ridx1) if options.ridx1 else sys.stdin as reader, \
            open_xz_or_gzip_or_plain(options.oridx, 'wt') if options.oridx else dummy_open() as oridx_writer:
        for i in reader:
            fields = i.split("\t")
            if len(fields) >= 2:
                fields_n = fields[1].split(":")
                doc1 = int(fields[0])
                doc2 = int(fields_n[1])
                score = float(fields_n[1])
                if score < options.threshold:
                    continue
                if oridx_writer:
                    oridx_writer.write(i)
                try:
                    indices[doc1] = doc2
                except:
                    pass
else:
    with open_xz_or_gzip_or_plain(options.ridx1, 'rt') as reader1, \
            open_xz_or_gzip_or_plain(options.ridx2, 'rt') as reader2, \
            open_xz_or_gzip_or_plain(options.oridx, 'wt') if options.oridx else dummy_open() as oridx_writer:

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

            # Reading the .ridx file with the preliminary alignment in one of the directions
            for line_ridx2 in reader2:
                fields = line_ridx2.strip().split("\t")

                if len(fields) < 2:
                    continue

                doc_id_2 = int(fields[0])

                del fields[0]

                num_candidates = min(len(fields), options.candidate_num)
                candidateIterations = list(range(0, num_candidates))

                for candidate_idx in candidateIterations:
                    field_n = fields[candidate_idx].split(":")

                    doc_id_1 = int(field_n[0])
                    score = float(field_n[1])

                    if doc_id_1 not in pairedDocs and doc_id_2 not in pairedDocs:
                        # Avoid pairing docs already paired in previous iterations of the algorithm,
                        # in case you specified the parameter

                        if doc_id_1 not in best_ridx2_inv:
                            best_ridx2_inv[doc_id_1] = {}
                            documentsFile2.add(doc_id_1)
                        best_ridx2_inv[doc_id_1][doc_id_2] = score

                        if doc_id_2 not in best_ridx2:
                            best_ridx2[doc_id_2] = {}

                        best_ridx2[doc_id_2][doc_id_1] = score

                    elif doc_id_1 in pairedDocs and num_candidates < len(fields):
                        # If the documents are already paired, we just ignored the read candidate
                        # and do another iteration
                        candidateIterations.append(num_candidates)
                        num_candidates = num_candidates + 1

            # finished reading ridx2

            # Reading the .ridx file with the preliminary alignment in the other direction
            # and combining this information with the previous one
            pairedDocsLine = set()
            candidateDocuments = []
            for line_ridx1 in reader1:
                new_candidate_list = {}
                fields = line_ridx1.strip().split("\t")
                doc_id_1 = int(fields[0])
                del fields[0]

                if len(fields) < 2:
                    continue

                num_candidates = min(len(fields), options.candidate_num)
                candidateIterations = list(range(0, num_candidates))

                for candidate_idx in candidateIterations:
                    field_n = fields[candidate_idx].split(":")

                    doc_id_2 = int(field_n[0])
                    score = float(field_n[1])

                    if doc_id_1 not in pairedDocs and doc_id_2 not in pairedDocs:

                        # Same check for already paired documents in several iterations
                        documentsFile2.add(doc_id_2)

                        if doc_id_1 in best_ridx2_inv and doc_id_2 in best_ridx2_inv[doc_id_1]:
                            average = (best_ridx2_inv[doc_id_1][doc_id_2] + score) / 2
                            # product = best_ridx2_inv[doc_id_1][doc_id_2] * score
                            # TODO: we should implement also the product of the score/probability as
                            # an option/parameter
                            if average >= options.threshold:
                                new_candidate_list[doc_id_2] = average
                        if options.nonsymmetric:
                            if doc_id_2 not in new_candidate_list:
                                new_candidate_list[doc_id_2] = score

                    elif doc_id_2 in pairedDocs and num_candidates < len(fields):
                        # Same check to keep iterating and ignoring already paired documents
                        candidateIterations.append(num_candidates)
                        num_candidates = num_candidates + 1

                if len(new_candidate_list) >= 1:
                    candidateDocuments.append(len(new_candidate_list))
                    sorted_candidates = sorted(iter(new_candidate_list.items()), key=itemgetter(1), reverse=True)
                    if doc_id_1 not in indicesProb or indicesProb[doc_id_1] < sorted_candidates[0][1]:
                        # We store the final indices and their scores, in case of any symmetric relationship overlap
                        indices[doc_id_1] = sorted_candidates[0][0]
                        indicesProb[doc_id_1] = sorted_candidates[0][1]
                    pairedDocsLine.add(doc_id_1)
                    pairedDocsLine.add(sorted_candidates[0][0])
                    if oridx_writer:
                        elements = [f"{doc_id}:{score}" for (doc_id, score) in sorted_candidates]
                        elements_string = "\t".join(elements)
                        oridx_writer.write(f"{doc_id_1}\t{elements_string}\n")

            # finished reading ridx1

            pairedDocs.update(pairedDocsLine)

            if options.nonsymmetric:
                for document in sorted(list(documents - pairedDocs)):
                    # We now print the relationships of the first read relationships file with unpaired documents
                    if document in best_ridx2 and len(best_ridx2[document]) >= 1:
                        new_candidate_list = best_ridx2[document]
                        candidateDocuments.append(len(new_candidate_list))
                        sorted_candidates = sorted(iter(new_candidate_list.items()), key=itemgetter(1), reverse=True)
                        if int(document) not in indicesProb \
                                or indicesProb[document] < sorted_candidates[0][1]:
                            indices[document] = sorted_candidates[0][0]
                            indicesProb[document] = sorted_candidates[0][1]
                        pairedDocs.add(document)
                        pairedDocs.add(sorted_candidates[0][0])
                        if oridx_writer:
                            elements = [f"{doc_id}:{score}" for (doc_id, score) in sorted_candidates]
                            for (document2, score) in elements:
                                oridx_writer.write("{0}\t{1}:{2}\n".format(document2, document, score))
            # End of the iterations if the result did not change from the previous iteration
            # (exit point of the endless loop)
            if len(candidateDocuments) == 0:
                break

for k in indices:
    if indices[k] in documents:
        if indices[k] in documentsFile2:
            # Write the output keeping the language documents always in the same order
            # (this is done because of the non-symmetric option that causes swaps in the last algorithm)
            print(f"{k}\t{indices[k] - file2_start_counter}")
        else:
            print(f"{k-file2_start_counter}\t{indices[k]}")
        if not options.nonsymmetric:
            documents.remove(k)