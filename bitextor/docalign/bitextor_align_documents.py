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

def process_candidates_ridx2(values, max_candidates, doc_pairs, best_ridx, best_ridx_inv):
    num_candidates = min(len(values), max_candidates)
    max_candidate_iterations = num_candidates
    candidate_iterations_idx = 0
    doc_id_2 = f"d2_{values[0][0]}"

    if len(values) == 0:
        return

    while candidate_iterations_idx < max_candidate_iterations:
        if doc_id_2[3:] != str(values[candidate_iterations_idx][0]):
            raise Exception("Unexpected different value")

        doc_id_1 = f"d1_{values[candidate_iterations_idx][1]}"
        score = values[candidate_iterations_idx][2]

        if doc_id_1 not in doc_pairs and doc_id_2 not in doc_pairs:
            # Avoid pairing docs already paired in previous iterations of the algorithm,
            # in case you specified the parameter
            if doc_id_1 not in best_ridx_inv:
                best_ridx_inv[doc_id_1] = {}

            best_ridx_inv[doc_id_1][doc_id_2] = score

            if doc_id_2 not in best_ridx:
                best_ridx[doc_id_2] = {}

            best_ridx[doc_id_2][doc_id_1] = score

        elif doc_id_1 in doc_pairs and num_candidates < len(values):
            # If the documents are already paired, we just ignored the read candidate
            # and do another iteration
            max_candidate_iterations += 1
            num_candidates += 1

        candidate_iterations_idx += 1

def process_candidates_ridx1(values, max_candidates, doc_pairs, best_ridx_inv, new_candidate_list, candidate_documents,
                             indices, indices_prob, oridx_writer, paired_docs, paired_docs_score, threshold, non_symmetric):
    num_candidates = min(len(values), max_candidates)
    max_candidate_iterations = num_candidates
    candidate_iterations_idx = 0
    doc_id_1 = f"d1_{values[0][0]}"

    if len(values) == 0:
        return

    while candidate_iterations_idx < max_candidate_iterations:
        if doc_id_1[3:] != str(values[candidate_iterations_idx][0]):
            raise Exception("Unexpected different value")

        doc_id_2 = f"d2_{values[candidate_iterations_idx][1]}"
        score = values[candidate_iterations_idx][2]

        if doc_id_1 not in doc_pairs and doc_id_2 not in doc_pairs:
            # Same check for already paired documents in several iterations

            if doc_id_1 in best_ridx_inv and doc_id_2 in best_ridx_inv[doc_id_1]:
                average = (best_ridx_inv[doc_id_1][doc_id_2] + score) / 2
                # product = best_ridx_inv[doc_id_1][doc_id_2] * score
                # TODO: we should implement also the product of the score/probability as
                # an option/parameter
                if average >= threshold:
                    new_candidate_list[doc_id_2] = average

            if non_symmetric:
                if doc_id_2 not in new_candidate_list and score >= threshold:
                    new_candidate_list[doc_id_2] = score

        elif doc_id_2 in doc_pairs and num_candidates < len(values):
            # Same check to keep iterating and ignoring already paired documents
            max_candidate_iterations += 1
            num_candidates += 1

        candidate_iterations_idx += 1

    if len(new_candidate_list) >= 1:
        candidate_documents.append(len(new_candidate_list))
        sorted_candidates = sorted(iter(new_candidate_list.items()), key=itemgetter(1), reverse=True)

        if doc_id_1 not in indices_prob or indices_prob[doc_id_1] < sorted_candidates[0][1]:
            # We store the final indices and their scores, in case of any symmetric relationship overlap
            indices[doc_id_1] = sorted_candidates[0][0]
            indices_prob[doc_id_1] = sorted_candidates[0][1]

        paired_docs.add(doc_id_1)
        paired_docs.add(sorted_candidates[0][0])
        paired_docs_score[f"{doc_id_1}+{sorted_candidates[0][0]}"] = sorted_candidates[0][1]

        if oridx_writer:
            for doc_id, score in sorted_candidates:
                if doc_id_1[:3] != "d1_" or doc_id[:3] != "d2_":
                    raise Exception(f"Unexpected document idxs: {doc_id_1} and {doc_id}")

                oridx_writer.write(f"{doc_id_1[3:]}\t{doc_id[3:]}\t{score}\n")

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

oparser.add_argument("--print-score", action="store_true",
                    help="Print score of the paired docs")

options = oparser.parse_args()

indices = {}
indicesProb = {}
documents = set(map(lambda x: f"d1_{x}", range(1, options.ndoc1 + 1))).\
                union(set(map(lambda x: f"d2_{x}", range(1, options.ndoc2 + 1))))

if options.ridx2 is None:
    # Reading the .ridx file with the preliminary alignment
    with open_xz_or_gzip_or_plain(options.ridx1) if options.ridx1 else sys.stdin as reader, \
            open_xz_or_gzip_or_plain(options.oridx, 'wt') if options.oridx else dummy_open() as oridx_writer:
        header = next(reader).strip().split('\t')
        src_doc_idx_idx = header.index("src_index")
        trg_doc_idx_idx = header.index("trg_index")
        score_idx = header.index("rank_score")

        # Print output header
        if oridx_writer:
            oridx_writer.write('\t'.join(header) + '\n')

        for i in reader:
            fields = i.strip().split('\t')
            src_doc_idx = int(fields[src_doc_idx_idx])
            trg_doc_idx = int(fields[trg_doc_idx_idx])
            score = float(fields[score_idx])

            if score < options.threshold:
                continue

            if oridx_writer:
                oridx_writer.write(i)

            try:
                indices[f"d1_{src_doc_idx}"] = f"d2_{trg_doc_idx}"
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

        iterations = int(iterations)
        current_iteration = 0
        pairedDocs = set()
        paired_docs_score = {}

        # In each iteration we read the whole file of document relationships and candidates,
        # but we ignore already paired documents
        while current_iteration < iterations:
            reader2.seek(0)
            reader1.seek(0)

            header2 = next(reader2).strip().split('\t')
            src_doc_idx_idx2 = header2.index("src_index")
            trg_doc_idx_idx2 = header2.index("trg_index")
            score_idx2 = header2.index("rank_score")
            header1 = next(reader1).strip().split('\t')
            src_doc_idx_idx1 = header1.index("src_index")
            trg_doc_idx_idx1 = header1.index("trg_index")
            score_idx1 = header1.index("rank_score")

            if converge:
                # We create an infinite loop and we will stop when the algorithm converges
                iterations += 1

            # We store both directions of document relationships for printing purposes
            best_ridx2_inv = {}
            best_ridx2 = {}

            last_doc_id_2 = -1
            reader2_values = []

            # Reading the .ridx file with the preliminary alignment in one of the directions
            for line_ridx2 in reader2:
                fields = line_ridx2.strip().split("\t")
                current_doc_id_2 = int(fields[src_doc_idx_idx2])
                current_doc_id_1 = int(fields[trg_doc_idx_idx2])
                current_score = float(fields[score_idx2])

                if last_doc_id_2 == -1:
                    last_doc_id_2 = current_doc_id_2

                if last_doc_id_2 != current_doc_id_2:
                    # Process
                    process_candidates_ridx2(reader2_values, options.candidate_num, pairedDocs, best_ridx2, best_ridx2_inv)

                    reader2_values = []

                reader2_values.append((current_doc_id_2, current_doc_id_1, current_score))

                last_doc_id_2 = current_doc_id_2

            if len(reader2_values) != 0:
                # Process
                process_candidates_ridx2(reader2_values, options.candidate_num, pairedDocs, best_ridx2, best_ridx2_inv)

                reader2_values = []

            # finished reading ridx2

            # Reading the .ridx file with the preliminary alignment in the other direction
            # and combining this information with the previous one
            pairedDocsLine = set()
            candidateDocuments = []
            new_candidate_list = {}
            last_doc_id_1 = -1
            reader1_values = []

            if current_iteration == 0 and oridx_writer:
                # Print output header
                oridx_writer.write("src_index\ttrg_index\tdocalign_score\n")

            for line_ridx1 in reader1:
                fields = line_ridx1.strip().split("\t")
                current_doc_id_1 = int(fields[src_doc_idx_idx1])
                current_doc_id_2 = int(fields[trg_doc_idx_idx1])
                current_score = float(fields[score_idx1])

                if last_doc_id_1 == -1:
                    last_doc_id_1 = current_doc_id_1

                if last_doc_id_1 != current_doc_id_1:
                    # Process
                    process_candidates_ridx1(reader1_values, options.candidate_num, pairedDocs, best_ridx2_inv, new_candidate_list,
                                             candidateDocuments, indices, indicesProb, oridx_writer, pairedDocsLine, paired_docs_score,
                                             options.threshold, options.nonsymmetric)

                    reader1_values = []

                reader1_values.append((current_doc_id_1, current_doc_id_2, current_score))

                last_doc_id_1 = current_doc_id_1
                new_candidate_list = {}

            if len(reader1_values) != 0:
                # Process
                process_candidates_ridx1(reader1_values, options.candidate_num, pairedDocs, best_ridx2_inv, new_candidate_list,
                                         candidateDocuments, indices, indicesProb, oridx_writer, pairedDocsLine, paired_docs_score,
                                         options.threshold, options.nonsymmetric)

                reader1_values = []

            # finished reading ridx1

            pairedDocs.update(pairedDocsLine)

            if options.nonsymmetric:
                for document in sorted(list(documents - pairedDocs)):
                    # We now print the relationships of the first read relationships file with unpaired documents
                    if document in best_ridx2 and len(best_ridx2[document]) >= 1:
                        new_candidate_list = best_ridx2[document]
                        candidateDocuments.append(len(new_candidate_list))
                        sorted_candidates = sorted(iter(new_candidate_list.items()), key=itemgetter(1), reverse=True)

                        if sorted_candidates[0][1] >= options.threshold:
                            if document not in indicesProb or indicesProb[document] < sorted_candidates[0][1]:
                                indices[document] = sorted_candidates[0][0]
                                indicesProb[document] = sorted_candidates[0][1]

                            pairedDocs.add(document)
                            pairedDocs.add(sorted_candidates[0][0])
                            paired_docs_score[f"{document}+{sorted_candidates[0][0]}"] = sorted_candidates[0][1] # d2 -> d1

                            if oridx_writer:
                                for (document2, score) in sorted_candidates:
                                    if document2[:3] != "d1_" or document[:3] != "d2_":
                                        raise Exception(f"Unexpected document idxs: {document2} and {document}")

                                    oridx_writer.write(f"{document2[3:]}\t{document[3:]}\t{score}\n")

            # End of the iterations if the result did not change from the previous iteration
            # (exit point of the endless loop)
            if len(candidateDocuments) == 0:
                break

            current_iteration += 1

# Print output header
sys.stdout.write("src_index\ttrg_index")

if options.print_score:
    sys.stdout.write("\tdic_doc_aligner_score")

sys.stdout.write('\n')

for k in indices:
    if indices[k] in documents:
        src_idx_doc, trg_idx_doc = None, None
        score = None
        score_idx = f"{k}+{indices[k]}"

        if k[:3] == "d1_" and indices[k][:3] == "d2_":
            src_idx_doc = k[3:]
            trg_idx_doc = indices[k][3:]
        elif k[:3] == "d2_" and indices[k][:3] == "d1_":
            src_idx_doc = indices[k][3:]
            trg_idx_doc = k[3:]
        else:
            raise Exception(f"Unexpected document idxs: {k} and {indices[k]}")

        if options.print_score and score_idx not in paired_docs_score:
            raise Exception(f"Could not find the score for the paired docs {src_idx_doc}-{trg_idx_doc}")

        sys.stdout.write(f"{src_idx_doc}\t{trg_idx_doc}")

        if options.print_score:
            score = paired_docs_score[score_idx]

            sys.stdout.write(f"\t{score}")

        sys.stdout.write('\n')

        if not options.nonsymmetric:
            documents.remove(k)
