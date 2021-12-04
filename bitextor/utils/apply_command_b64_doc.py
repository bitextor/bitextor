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

import sys
import shlex
import base64
import logging
import argparse
import contextlib
import subprocess

def execute(command, input_file='-', use_shell=False, remove_empty_docs=False, empty_docs_value='Cg==',
            is_plaintext=False):
    command = command if use_shell else shlex.split(command)
    non_empty_docs = 0
    empty_docs = 0
    processed_sentences = 0
    total_sentences = 0
    cm = contextlib.nullcontext(sys.stdin)

    if input_file != '-':
        cm = open(input_file)

    with cm as doc_fd:
        for idx, doc in enumerate(doc_fd):
            try:
                doc = doc.strip()

                if is_plaintext:
                    sentences = doc.encode('utf-8')
                else:
                    sentences = base64.b64decode(doc)

                    total_sentences += sentences.decode('utf-8').strip().count('\n') + 1

                # Execute command for the sentences of the current document, BASE64-decoded
                command_result = subprocess.Popen(command, shell=use_shell, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
                sentences = command_result.communicate(sentences)[0]

                processed_sentences += sentences.decode('utf-8').strip().count('\n') + 1

                sentences = base64.b64encode(sentences).decode("utf-8")

                if sentences.strip() != '':
                    print(sentences)
                    non_empty_docs += 1
                elif not remove_empty_docs:
                    # Print, at least, a minimum document content -> #input documents = #output documents
                    print(empty_docs_value)
                    empty_docs += 1
                else:
                    empty_docs += 1

            except Exception as e:
                raise Exception(f"doc #{idx + 1} could not be processed") from e

    logging.debug("%d documents were empty, and %d were not", empty_docs, non_empty_docs)
    logging.debug("%d sentences were returned (initial sentences: %d)", processed_sentences - (0 if remove_empty_docs else empty_docs), total_sentences)

def parse_args():
    parser = argparse.ArgumentParser(description="It processes BASE64 documents with a provided command and returns the BASE64-encoded result",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('command',
                        help="Command which will be executed")
    parser.add_argument('--input', default='-',
                        help="Input file. If '-' is provided, stdin will be used")
    parser.add_argument('--input-is-not-base64', action='store_true',
                        help="Input is expected to be BASE64-encoded text, but with this option plaintext will be expected instead")
    parser.add_argument('--use-shell', action='store_true',
                        help="When using shell=True with subprocess, we lose control but the evaluation of the provided command is literal, so"
                             "even pipes can be used")
    parser.add_argument('--remove-empty-docs', action='store_true',
                        help="By default, empty documents, result of the provided command, are kept")
    parser.add_argument('--empty-docs-value', default='Cg==',
                        help="Default value to use when an empty document is produced, result of the provided command. It should be a BASE64 value")
    parser.add_argument('--logging-level', type=int, default=logging.INFO,
                        help="Logging level")

    args = parser.parse_args()

    return args

if __name__ == '__main__':
    args = parse_args()

    logging.basicConfig(level=args.logging_level)

    execute(args.command, input_file=args.input, use_shell=args.use_shell, remove_empty_docs=args.remove_empty_docs,
            empty_docs_value=args.empty_docs_value, is_plaintext=args.input_is_not_base64)
