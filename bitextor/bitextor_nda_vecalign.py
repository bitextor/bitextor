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

# WARNING: this script has been developed to work with https://github.com/bitextor/vecalign (fork from the original Vecalign)

import os
import sys
import shlex
import base64
import logging
import argparse
import subprocess

from utils.common import get_all_idxs_from_list

logger = logging

def get_full_path(path):
    return os.path.realpath(os.path.expanduser(path))

def preprocess_file_content(content, return_list=False):
    result = filter(lambda line: line != "", map(lambda line: line.strip(), content.split("\n") if isinstance(content, str) else content))

    if return_list:
        return list(result)

    return result

def process_nda_output(input_file, output_file, input_is_base64=False, first_match_offset=0):
    src_sentences, trg_sentences = [], []
    src_urls, trg_urls = [], []
    src_idxs, trg_idxs = [], []

    # Read the output file
    output_header = next(output_file).strip().split('\t')
    output = preprocess_file_content([l for l in output_file])

    if len(output_header) not in (2, 3):
        raise Exception(f"unexpected NDA output format. Expected columns was 3|2, got {len(values)}")

    src_idx_idx = output_header.index("src_idx")
    trg_idx_idx = output_header.index("trg_idx")

    # Process the output
    for row in output:
        values = row.split("\t")

        try:
            src_idx, trg_idx = int(values[src_idx_idx]) - first_match_offset, int(values[trg_idx_idx]) - first_match_offset

            src_idxs.append(src_idx)
            trg_idxs.append(trg_idx)
        except ValueError as e:
            raise Exception("could not parse the columns from the NDA output to int (wrong format?)") from e

    if len(src_idxs) != len(trg_idxs):
        raise Exception("different number of src and trg documents while processing NDA output")

    # Get the documents
    total_src_files, total_trg_files = 0, 0
    index = {"src_sentences": [], "trg_sentences": [],
             "src_urls": [], "trg_urls": [],
             "src_matches": {}, "trg_matches": {},
             "src_matches_index": {}, "trg_matches_index": {},
             "matches": set()}

    for line in input_file:
        line = line.strip().split("\t")
        sentences = None

        if len(line) != 3:
            raise Exception(f"unexpected NDA input format: expected columns was 3, got {len(line)}")

        # Get Base64 value
        if input_is_base64:
            sentences = preprocess_file_content(line[0], return_list=True)

            if len(sentences) != 1:
                raise Exception("unexpected length after reading base64 entry: expected length "
                                f"was 1, got {len(sentences)}")

            sentences = sentences[0]

        # Read the files
        else:
            with open(line[0]) as doc:
                sentences = base64.b64encode("\n".join(preprocess_file_content(doc.readlines())).encode("utf-8")).decode("utf-8")

        if line[2] == "src":
            # Check if the current document is one of the results from the output file
            if total_src_files in src_idxs:
                # Create entry in the index
                index["src_sentences"].append(sentences)
                index["src_urls"].append(line[1])
                index["src_matches"][total_src_files] = set()
                index["src_matches_index"][total_src_files] = len(index["src_sentences"]) - 1

                for idx in get_all_idxs_from_list(src_idxs, total_src_files):
                    index["matches"].add((src_idxs[idx], trg_idxs[idx]))
                    index["src_matches"][total_src_files].add(trg_idxs[idx])

            total_src_files += 1
        elif line[2] == "trg":
            # Check if the current document is one of the results from the output file
            if total_trg_files in trg_idxs:
                # Create entry in the index
                index["trg_sentences"].append(sentences)
                index["trg_urls"].append(line[1])
                index["trg_matches"][total_trg_files] = set()
                index["trg_matches_index"][total_trg_files] = len(index["trg_sentences"]) - 1

                for idx in get_all_idxs_from_list(trg_idxs, total_trg_files):
                    index["matches"].add((src_idxs[idx], trg_idxs[idx]))
                    index["trg_matches"][total_trg_files].add(src_idxs[idx])

            total_trg_files += 1
        else:
            raise Exception(f"unexpected NDA input format: expected 3rd column was src|trg, got {line[2]}")

    # Process index in order to obtain the sentences and URLs sorted
    for src_idx, trg_idx in index["matches"]:
        src_sentence_idx = index["src_matches_index"][src_idx]
        src_sentence = index["src_sentences"][src_sentence_idx]
        src_url = index["src_urls"][src_sentence_idx]

        trg_sentence_idx = index["trg_matches_index"][trg_idx]
        trg_sentence = index["trg_sentences"][trg_sentence_idx]
        trg_url = index["trg_urls"][trg_sentence_idx]

        src_sentences.append(src_sentence)
        trg_sentences.append(trg_sentence)
        src_urls.append(src_url)
        trg_urls.append(trg_url)

        # Free resources if possible
        index["src_matches"][src_idx].remove(trg_idx)
        index["trg_matches"][trg_idx].remove(src_idx)

        # TODO is it ok?
        #if len(index["src_matches"][src_idx]) == 0:
        #    del index["src_sentences"][src_sentence_idx]
        #    del index["src_urls"][src_sentence_idx]
        #    del index["src_matches"][src_idx]
        #    del index["src_matches_index"][src_idx]
        #if len(index["trg_matches"][trg_idx]) == 0:
        #    del index["trg_sentences"][trg_sentence_idx]
        #    del index["trg_urls"][trg_sentence_idx]
        #    del index["trg_matches"][trg_idx]
        #    del index["trg_matches_index"][trg_idx]

    return src_sentences, trg_sentences, src_urls, trg_urls

def vecalign_overlap(base64_input_list, overlaps_output_path, num_overlaps, paragraphs=False):
    if os.path.isfile(overlaps_output_path):
        # Do not generate overlapping file because already exists
        return

    paragraphs = ["--paragraph_identification"] if paragraphs else []

    # Generate overlapping file
    result = subprocess.Popen(["vecalign-overlap", "-i", "-", "-o", overlaps_output_path, "-n", str(num_overlaps), *paragraphs],
                              stdin=subprocess.PIPE, stdout=None, stderr=None)

    result.communicate(input=("\n".join(base64_input_list)).encode("utf-8"))

    if result.returncode != 0:
        raise Exception(f"something went wrong while generating the overlapping files for Vecalign: return code is {result.returncode}")

    if not os.path.isfile(overlaps_output_path):
        raise Exception(f"overlap file {overlaps_output_path} should exist, but it does not exist")

def main(args):
    nda_input_path = args.nda_input_path
    nda_output_path = args.nda_output_path
    tmp_dir = args.tmp_dir
    nda_input_is_base64 = args.nda_input_is_base64
    vecalign_num_overlaps = args.vecalign_num_overlaps
    alignment_max_size = args.vecalign_alignment_max_size
    dim = args.dim
    embeddings_batch_size = args.embeddings_batch_size
    src_overlapping = args.src_overlapping
    trg_overlapping = args.trg_overlapping
    src_embedding = args.src_embedding
    trg_embedding = args.trg_embedding
    first_match_offset = args.first_match_offset
    paragraph_identification = args.paragraph_identification
    sent_hash_cmd = args.print_sent_hash
    model = args.model

    if (nda_input_path == sys.stdin and nda_output_path == sys.stdin):
        raise Exception("you can only pipe either nda input or nda output, not both of them")

    vecalign_overlaps_src_path = f"{tmp_dir}/overlaps.src" if src_overlapping is None else src_overlapping
    vecalign_overlaps_trg_path = f"{tmp_dir}/overlaps.trg" if trg_overlapping is None else trg_overlapping
    vecalign_overlaps_src_embeddings_path = f"{tmp_dir}/overlaps.emb.src" if src_embedding is None else src_embedding
    vecalign_overlaps_trg_embeddings_path = f"{tmp_dir}/overlaps.emb.trg" if trg_embedding is None else trg_embedding

    if not os.path.isdir(tmp_dir):
        raise Exception(f"temporal directory does not exist: {tmp_dir}")

    # Process output from NDA. Returned sentences are Base64 values where each Base64 entry is a document
    src_sentences, trg_sentences, src_urls, trg_urls = process_nda_output(nda_input_path, nda_output_path, nda_input_is_base64,
                                                                          first_match_offset=first_match_offset)

    if (len(src_sentences) != len(trg_sentences) or len(trg_sentences) != len(src_urls) or len(src_urls) != len(trg_urls)):
        raise Exception("unexpected lengths from [src, trg] sentences and [src, trg] URLs (all them should match): "
                        f"{len(src_sentences)} vs {len(trg_sentences)} vs {len(src_urls)} vs {len(trg_urls)}")

    # Generate overlapping files
    vecalign_overlap(src_sentences, vecalign_overlaps_src_path, vecalign_num_overlaps, paragraphs=paragraph_identification)
    vecalign_overlap(trg_sentences, vecalign_overlaps_trg_path, vecalign_num_overlaps, paragraphs=paragraph_identification)

    # Execute vecalign (it will generate the embeddings and/or overlapping files if they do not exist)
    threshold = ["--threshold", str(args.threshold)] if args.threshold is not None else []
    storage_flags = []

    # Generate storage flags
    if (args.embedding_src_storage_input is not None and args.embedding_src_storage_path is not None):
        storage_flags.extend(["--embeddings_src_storage_input", args.embedding_src_storage_input,
                              "--embeddings_src_storage_path", args.embedding_src_storage_path])

        if args.embedding_src_storage_input_base64:
            storage_flags.append("--embeddings_src_storage_input_base64")

        logger.info("using embeddings storage (src)")
    if (args.embedding_trg_storage_input is not None and args.embedding_trg_storage_path is not None):
        storage_flags.extend(["--embeddings_tgt_storage_input", args.embedding_trg_storage_input,
                              "--embeddings_tgt_storage_path", args.embedding_trg_storage_path])

        if args.embedding_trg_storage_input_base64:
            storage_flags.append("--embeddings_tgt_storage_input_base64")

        logger.info("using embeddings storage (trg)")

    if args.embedding_src_storage_not_uniq:
        storage_flags.append("--embeddings_src_storage_are_not_uniq")
    if args.embedding_trg_storage_not_uniq:
        storage_flags.append("--embeddings_tgt_storage_are_not_uniq")

    storage_flags.extend(["--src_embeddings_optimization_strategy", str(args.src_embeddings_optimization_strategy),
                          "--tgt_embeddings_optimization_strategy", str(args.trg_embeddings_optimization_strategy),
                          "--src_storage_embeddings_optimization_strategy", str(args.src_storage_embeddings_optimization_strategy),
                          "--tgt_storage_embeddings_optimization_strategy", str(args.trg_storage_embeddings_optimization_strategy)])
    paragraphs = ["--paragraph_identification"] if paragraph_identification else []
    model_param = ["--embeddings_model", model] if model else []

    result = subprocess.Popen(["vecalign", "--alignment_max_size", str(alignment_max_size),
                               "--src", "-", "--tgt", "-", "--src_urls", "-", "--tgt_urls", "-",
                               "--src_embed", vecalign_overlaps_src_path, vecalign_overlaps_src_embeddings_path,
                               "--tgt_embed", vecalign_overlaps_trg_path, vecalign_overlaps_trg_embeddings_path,
                               *threshold, "--embeddings_dim", str(dim), "--urls_format",
                               "--embeddings_batch_size", str(embeddings_batch_size), *storage_flags,
                               *paragraphs, *model_param],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)

    # Pipe input and get output
    input_base64 = "\n".join([f"{a}\t{b}\t{c}\t{d}" for a, b, c, d in zip(src_sentences, trg_sentences, src_urls, trg_urls)])
    stdout, _ = result.communicate(input=input_base64.encode("utf-8"))

    if result.returncode != 0:
        raise Exception(f"something went wrong while running vecalign: return code is {result.returncode}")

    stdout = stdout.decode("utf-8").rstrip('\n')
    header = stdout[:stdout.find('\n')].split('\t')
    src_text_idx = header.index("src_text")
    trg_text_idx = header.index("trg_text")

    logger.debug("src and trg text idxs: %d %d", src_text_idx, trg_text_idx)

    if sent_hash_cmd:
        # Print deferred hashes
        sent_hash_cmd = shlex.split(sent_hash_cmd)
        stdout = stdout.split('\n')[1:]

        header.append("src_deferred_hash")
        header.append("trg_deferred_hash")

        print('\t'.join(header))

        for s in stdout:
            s = s.rstrip('\n').split('\t')
            src_text = s[src_text_idx]
            trg_text = s[trg_text_idx]
            src_deferred = subprocess.Popen(sent_hash_cmd, encoding="utf-8", stdin=subprocess.PIPE, stdout=subprocess.PIPE)\
                                     .communicate(src_text)[0].rstrip('\n')
            trg_deferred = subprocess.Popen(sent_hash_cmd, encoding="utf-8", stdin=subprocess.PIPE, stdout=subprocess.PIPE)\
                                     .communicate(trg_text)[0].rstrip('\n')

            s.append(src_deferred)
            s.append(trg_deferred)

            print('\t'.join(s))
    else:
        print(stdout)

def parse_args():
    parser = argparse.ArgumentParser(description='NDA output process for Vecalign')

    parser.add_argument('nda_input_path', metavar='nda-input-path', type=argparse.FileType('rt'),
                        help='Path to the input file of NDA')
    parser.add_argument('nda_output_path', metavar='nda-output-path', type=argparse.FileType('rt'),
                        help='Path to the output file of NDA with the results')

    # Other options
    parser.add_argument('--tmp-dir', metavar='PATH', required=True,
                        help='Path to tmp directory')
    parser.add_argument('--nda-input-is-base64', action='store_true',
                        help='If the nda input file contains the first row with Base64 instead of paths to documents')
    parser.add_argument('--first-match-offset', type=int, default=0,
                        help='The matches are expected to begin with zero, but if they begin with other value, the offset can be set with this flag')
    parser.add_argument('--print-sent-hash',
                        help='Provide command for a shasum like program to print MurmurHash hashes of the src and trg sentences')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Verbose logging output')
    ## Storage
    parser.add_argument('--embedding-src-storage-input', type=str,
                        help='Path to the src storage file which contains sentences. You will need to provide --embedding-src-storage-path as well')
    parser.add_argument('--embedding-src-storage-input-base64', action='store_true',
                        help='Sentences provided via --embedding-src-storage-input are base64 encoded')
    parser.add_argument('--embedding-src-storage-path', type=str,
                        help='Path to the src storage file which contains embeddings. You will need to provide --embedding-src-storage-input as well')
    parser.add_argument('--embedding-trg-storage-input', type=str,
                        help='Path to the trg storage file which contains sentences. You will need to provide --embedding-trg-storage-path as well')
    parser.add_argument('--embedding-trg-storage-input-base64', action='store_true',
                        help='Sentences provided via --embedding-trg-storage-input are base64 encoded')
    parser.add_argument('--embedding-trg-storage-path', type=str,
                        help='Path to the trg storage file which contains embeddings. You will need to provide --embedding-trg-storage-input as well')
    parser.add_argument('--embedding-src-storage-not-uniq', action='store_true',
                        help='Expected src storage embeddings are monotonic and unique (i.e. embeddings from previous sentences are not expected to be provided). If the provided embeddings are 1-1 with the sentences, this flag must be set')
    parser.add_argument('--embedding-trg-storage-not-uniq', action='store_true',
                        help='Expected trg storage embeddings are monotonic and unique (i.e. embeddings from previous sentences are not expected to be provided). If the provided embeddings are 1-1 with the sentences, this flag must be set')
    ## Optimization
    parser.add_argument('--src-embeddings-optimization-strategy', type=int, default=0, choices=[0, 1, 2],
                        help='Optimization strategy applied to the embeddings when being generated. The generated embeddings will be stored applying the same strategy')
    parser.add_argument('--trg-embeddings-optimization-strategy', type=int, default=0, choices=[0, 1, 2],
                        help='Optimization strategy applied to the embeddings when being generated. The generated embeddings will be stored applying the same strategy')
    parser.add_argument('--src-storage-embeddings-optimization-strategy', type=int, default=0, choices=[0, 1, 2],
                        help='Optimization strategy applied to the storage embeddings when being loaded')
    parser.add_argument('--trg-storage-embeddings-optimization-strategy', type=int, default=0, choices=[0, 1, 2],
                        help='Optimization strategy applied to the storage embeddings when being loaded')



    # Vecalign specific options
    parser.add_argument('--vecalign-num-overlaps', type=int, default=4,
                        help='Number of overlaps to apply to every sentence when using Vecalign. The default value is 4')
    parser.add_argument('--vecalign-alignment-max-size', type=int, default=4,
                        help='Max. size for alignments when using Vecalign. If the overlapping files do not exist, they will be generated with the number of overlaps specified here. The default value is 4')
    parser.add_argument('--threshold', type=float, default=None,
                        help='Threshold for the sentences which Vecalign matches')
    parser.add_argument('--dim', type=int, default=768,
                        help='Dimension of the embeddings. The default value is 768')
    parser.add_argument('--embeddings-batch-size', type=int, default=32,
                        help='Batch size when generating embeddings with Vecalign. The default value is 32')
    parser.add_argument('--src-overlapping', type=str,
                        help='Source overlapping file. If does not exist, it will be generated')
    parser.add_argument('--trg-overlapping', type=str,
                        help='Target overlapping file. If does not exist, it will be generated')
    parser.add_argument('--src-embedding', type=str,
                        help='Source embedding file. If does not exist, it will be generated')
    parser.add_argument('--trg-embedding', type=str,
                        help='Target embedding file. If does not exist, it will be generated')
    parser.add_argument('--paragraph-identification', action='store_true',
                        help='Enable paragraph identification')
    parser.add_argument('--model',
                        help='Model from SentenceTransformers to use in order to generate/load the embeddings')

    args = parser.parse_args()

    return args

# This function should only be invoked when running as script and not as module
def main_wrapper():
    args = parse_args()

    logger = logging.getLogger("nda_vecalign")

    logger.setLevel(logging.DEBUG if args.verbose else logging.INFO)

    main(args)

if __name__ == "__main__":
    main_wrapper()
