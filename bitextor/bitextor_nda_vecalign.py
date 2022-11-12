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
import shlex
import logging
import argparse
import subprocess

logger = logging

def process_input_data(input_file, metadata_header_fields=''):
    src_sentences, trg_sentences = [], []
    src_urls, trg_urls = [], []
    src_metadata, trg_metadata = [], []
    metadata = True if metadata_header_fields else False

    # Header
    header = next(input_file).strip().split('\t')
    src_url_idx = header.index("src_url")
    trg_url_idx = header.index("trg_url")
    src_text_idx = header.index("src_text")
    trg_text_idx = header.index("trg_text")
    src_metadata_idx = header.index("src_metadata") if metadata else None
    trg_metadata_idx = header.index("trg_metadata") if metadata else None

    for line in input_file:
        line = line.rstrip('\n').split("\t")

        src_url = line[src_url_idx]
        trg_url = line[trg_url_idx]
        src_text = line[src_text_idx]
        trg_text = line[trg_text_idx]

        src_urls.append(src_url)
        trg_urls.append(trg_url)
        src_sentences.append(src_text)
        trg_sentences.append(trg_text)

        if metadata:
            src_meta = line[src_metadata_idx]
            trg_meta = line[trg_metadata_idx]

            src_metadata.append(src_meta)
            trg_metadata.append(trg_meta)

    return src_sentences, trg_sentences, src_urls, trg_urls, src_metadata, trg_metadata

def vecalign_overlap(base64_input_list, overlaps_output_path, num_overlaps):
    if os.path.isfile(overlaps_output_path):
        # Do not generate overlapping file because already exists
        return

    # Generate overlapping file
    result = subprocess.Popen(["vecalign-overlap", "-i", "-", "-o", overlaps_output_path, "-n", str(num_overlaps)],
                              stdin=subprocess.PIPE, stdout=None, stderr=None)

    result.communicate(input=("\n".join(base64_input_list)).encode("utf-8"))

    if result.returncode != 0:
        raise Exception(f"something went wrong while generating the overlapping files for Vecalign: return code is {result.returncode}")

    if not os.path.isfile(overlaps_output_path):
        raise Exception(f"overlap file {overlaps_output_path} should exist, but it does not exist")

def main(args):
    input_path = args.input_path
    tmp_dir = args.tmp_dir
    vecalign_num_overlaps = args.vecalign_num_overlaps
    alignment_max_size = args.vecalign_alignment_max_size
    dim = args.dim
    embeddings_batch_size = args.embeddings_batch_size
    src_overlapping = args.src_overlapping
    trg_overlapping = args.trg_overlapping
    src_embedding = args.src_embedding
    trg_embedding = args.trg_embedding
    sent_hash_cmd = args.print_sent_hash
    model = args.model
    metadata_header_fields = args.metadata_header_fields
    metadata = True if metadata_header_fields else False

    # Vecalign files
    vecalign_overlaps_src_path = f"{tmp_dir}/overlaps.src" if src_overlapping is None else src_overlapping
    vecalign_overlaps_trg_path = f"{tmp_dir}/overlaps.trg" if trg_overlapping is None else trg_overlapping
    vecalign_overlaps_src_embeddings_path = f"{tmp_dir}/overlaps.emb.src" if src_embedding is None else src_embedding
    vecalign_overlaps_trg_embeddings_path = f"{tmp_dir}/overlaps.emb.trg" if trg_embedding is None else trg_embedding

    if not os.path.isdir(tmp_dir):
        raise Exception(f"temporal directory does not exist: {tmp_dir}")

    # Process output from NDA. Returned sentences are Base64 values where each Base64 entry is a document
    src_sentences, trg_sentences, src_urls, trg_urls, src_metadata, trg_metadata = \
        process_input_data(input_path, metadata_header_fields=metadata_header_fields)

    if (len(src_sentences) != len(trg_sentences) or
        len(src_urls) != len(trg_urls) or
        len(trg_sentences) != len(src_urls)):
        raise Exception("unexpected lengths from [src, trg] sentences and [src, trg] URLs (all them should match): "
                        f"{len(src_sentences)} vs {len(trg_sentences)} vs {len(src_urls)} vs {len(trg_urls)}")
    if metadata and (len(src_metadata) != len(trg_metadata) or len(trg_sentences) != len(src_metadata)):
        raise Exception("unexpected lengths from [src, trg] sentences and [src, trg] metadata (all them should match): "
                        f"{len(src_sentences)} vs {len(trg_sentences)} vs {len(src_metadata)} vs {len(trg_metadata)}")

    # Generate overlapping files
    vecalign_overlap(src_sentences, vecalign_overlaps_src_path, vecalign_num_overlaps)
    vecalign_overlap(trg_sentences, vecalign_overlaps_trg_path, vecalign_num_overlaps)

    # Execute vecalign (it will generate the embeddings and/or overlapping files if they do not exist)
    threshold = ["--threshold", str(args.threshold)] if args.threshold is not None else []
    storage_flags = []

    # Generate storage flags
    if (args.embedding_src_storage_input is not None and args.embedding_src_storage_path is not None):
        storage_flags.extend(["--embeddings_src_storage_input", args.embedding_src_storage_input,
                              "--embeddings_src_storage_path", args.embedding_src_storage_path])

        # Documents are expected in BASE64
        storage_flags.append("--embeddings_src_storage_input_base64")

        logger.info("using embeddings storage (src)")
    if (args.embedding_trg_storage_input is not None and args.embedding_trg_storage_path is not None):
        storage_flags.extend(["--embeddings_tgt_storage_input", args.embedding_trg_storage_input,
                              "--embeddings_tgt_storage_path", args.embedding_trg_storage_path])

        # Documents are expected in BASE64
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
    metadata_args = ["--metadata_header_fields", metadata_header_fields] if metadata else []
    model_param = ["--embeddings_model", model] if model else []

    result = subprocess.Popen(["vecalign", "--alignment_max_size", str(alignment_max_size),
                               "--read_from_stdin",
                               "--src_embed", vecalign_overlaps_src_path, vecalign_overlaps_src_embeddings_path,
                               "--tgt_embed", vecalign_overlaps_trg_path, vecalign_overlaps_trg_embeddings_path,
                               *threshold, "--embeddings_dim", str(dim), "--urls_format",
                               "--embeddings_batch_size", str(embeddings_batch_size), *storage_flags,
                               *metadata_args, *model_param],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)

    # Pipe input and get output
    if metadata:
        input_base64 = "\n".join([f"{a}\t{b}\t{c}\t{d}\t{e}\t{f}" for a, b, c, d, e, f in zip(src_sentences, trg_sentences, src_urls, trg_urls, src_metadata, trg_metadata)])
    else:
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
        metadata_fields = metadata_header_fields.split(',') if metadata else []
        metadata_fields_dict = {}

        header.append("src_deferred_hash")
        header.append("trg_deferred_hash")

        if metadata:
            for field in metadata_fields:
                header.append(f"src_{field}")
                header.append(f"trg_{field}")

                metadata_fields_dict[f"src_{field}"] = len(header) - 2
                metadata_fields_dict[f"trg_{field}"] = len(header) - 1

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

            if metadata:
                for field in metadata_fields:
                    s.append(s[metadata_fields_dict[f"src_{field}"]])
                    s.append(s[metadata_fields_dict[f"trg_{field}"]])

            print('\t'.join(s))
    else:
        print(stdout)

def parse_args():
    parser = argparse.ArgumentParser(description='NDA output process for Vecalign')

    parser.add_argument('input_path', type=argparse.FileType('rt'),
                        help='Path to the data. Header is expected')

    # Other options
    parser.add_argument('--tmp-dir', metavar='PATH', required=True,
                        help='Path to tmp directory')
    parser.add_argument('--print-sent-hash',
                        help='Provide command for a shasum like program to print MurmurHash hashes of the src and trg sentences')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Verbose logging output')
    ## Storage
    parser.add_argument('--embedding-src-storage-input', type=str,
                        help='Path to the src storage file which contains documents in BASE64. You will need to provide --embedding-src-storage-path as well')
    parser.add_argument('--embedding-src-storage-path', type=str,
                        help='Path to the src storage file which contains embeddings. You will need to provide --embedding-src-storage-input as well')
    parser.add_argument('--embedding-trg-storage-input', type=str,
                        help='Path to the trg storage file which contains documents in BASE64. You will need to provide --embedding-trg-storage-path as well')
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
    parser.add_argument('--model',
                        help='Model from SentenceTransformers to use in order to generate/load the embeddings')
    parser.add_argument('--metadata-header-fields', type=str,
                        help='Provide language agnostic comma separated header fields if metadata is provided in the input. If provided, metadata will be processed')

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
