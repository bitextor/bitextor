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


# 1. Reading from STDIN a set of aligned documents. The input format is:
#   filename1	filename2	clean_text1_in_base64	clean_text2_in_base64
# 2. Text is cleaned and, for every aligned pair, both texts are dumped, in the same order in two temporary files.
# Every text block is sepparated to the previous one by a block:
#    <p>
#    <file lang="lang_id">file_name</file>
#    <p>
# 3. Running hunalign on the two temporary files
# 4. Removing unaligned segments and <p> mark
# 5. Identifying the filenames for every block of segments, and printing everything to the output
#
# Output format:
#   filename1    filename2    segment1    segment2    quality
#

# Notice that hunalign IS monotonic

import sys
import os
import argparse
import base64
import subprocess
import math
from tempfile import NamedTemporaryFile


def run_aligner(filename_s, filename_t, dic, hunalign_bin, threshold=0):
    # TODO option -ppthresh=10?
    hunalign_bin = hunalign_bin if hunalign_bin else f"{os.path.dirname(os.path.abspath(__file__))}/hunalign"
    dic = dic if dic else "/dev/null"

    # Optional args
    hunalign = [
        hunalign_bin,
        "-realign",
        f"-thresh={str(int(threshold))}"
    ]

    # Mandatory args
    hunalign.extend([
        dic,
        filename_s,
        filename_t
    ])

    p = subprocess.Popen(hunalign, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

    for line_o in p.stdout:
        yield line_o # Return generator

    return


def align(document_id_1, document_id_2, tokenized_file_1, tokenized_file_2, text_file_1, text_file_2, dic,
          hashprogram, paragraph_id, threshold=0.0):
    filereader1 = open(text_file_1, "r")
    filereader2 = open(text_file_2, "r")
    threshold = min(max(int(float(threshold) * 100.0), 0), 100) # hunalign threshold is [0, 100]
    hunalign_output = run_aligner(tokenized_file_1, tokenized_file_2, dic, options.hunalign_bin,
                                  threshold=threshold)

    # Read first hunalign match
    try:
        prev_hun = next(hunalign_output).strip()
        prev_fields = prev_hun.split(b"\t")

        # Try to get the nth line in order to check that, at least, there are n lines
        for _ in range(int(prev_fields[0])):
            filereader1.readline()

        for _ in range(int(prev_fields[1])):
            filereader2.readline()

    except StopIteration:
        # Hunalign couldn't find matches (the loop will not be executed since there are no lines)
        pass

    # Process hunalign output
    for line_h in hunalign_output:
        hun_line = line_h.strip()
        last_position1 = filereader1.tell()
        last_position2 = filereader2.tell()
        line1 = filereader1.readline().strip()
        line2 = filereader2.readline().strip()
        para_id_1 = ""
        para_id_2 = ""

        if paragraph_id:
            line1 = line1.split('\t')
            line2 = line2.split('\t')
            para_id_1 = line1[1]
            para_id_2 = line2[1]
            line1 = line1[0].strip()
            line2 = line2[0].strip()

        if hashprogram:
            hash1 = subprocess.run(
                [hashprogram],
                stdout=subprocess.PIPE,
                input=line1,
                encoding='utf8').stdout.rstrip('\n')
            hash2 = subprocess.run(
                [hashprogram],
                stdout=subprocess.PIPE,
                input=line2,
                encoding='utf8').stdout.rstrip('\n')

        prev_fields = prev_hun.split(b"\t")
        hunalign_fields = hun_line.split(b"\t")

        if math.isclose(float(prev_fields[2]), -0.3):
            # Skip match if the current match it is the same that the previous one (and hunalign said that has to be ignored)

            # TODO is this right? shouldn't be applied the same condition to both fields instead of just 1? continue after condition applied?
            if int(hunalign_fields[0]) == int(prev_fields[0]):
                line1 = ""
                hash1 = ""
                para_id_1 = ""
                filereader1.seek(last_position1)
            elif int(hunalign_fields[1]) == int(prev_fields[1]):
                line2 = ""
                hash2 = ""
                para_id_2 = ""
                filereader2.seek(last_position2)

        # We join all the lines between the last match and the current if necessary (we assume that
        #  if the previous line and the current are aligned, the ones in the middle are aligned as well)
        if int(hunalign_fields[0]) - int(prev_fields[0]) > 1:
            for _ in range((int(hunalign_fields[0]) - int(prev_fields[0])) - 1):
                tmp = filereader1.readline().strip()

                if paragraph_id:
                    tmp = tmp.split('\t')
                    para_id_1 += f"+{tmp[1]}"
                    tmp = tmp[0].strip()

                if hashprogram:
                    hash1 += "+" + subprocess.run([hashprogram], stdout=subprocess.PIPE,
                                                  input=tmp, encoding='utf8').stdout.rstrip('\n')

                line1 += " " + tmp

        if int(hunalign_fields[1]) - int(prev_fields[1]) > 1:
            for _ in range((int(hunalign_fields[1]) - int(prev_fields[1])) - 1):
                tmp = filereader2.readline().strip()

                if paragraph_id:
                    tmp = tmp.split('\t')
                    para_id_2 += f"+{tmp[1]}"
                    tmp = tmp[0].strip()

                if hashprogram:
                    hash2 += "+" + subprocess.run([hashprogram], stdout=subprocess.PIPE,
                                                  input=tmp, encoding='utf8').stdout.rstrip('\n')

                line2 += " " + tmp

        hunalign_score = prev_fields[2].decode("utf8")

        sys.stdout.write(f"{document_id_1}\t{document_id_2}\t{line1}\t{line2}\t{hunalign_score}")

        if paragraph_id:
            sys.stdout.write(f"\t{para_id_1}\t{para_id_2}")

        if hashprogram:
            sys.stdout.write(f"\t{hash1}\t{hash2}")

        sys.stdout.write('\n')

        prev_hun = hun_line

    filereader1.close()
    filereader2.close()


oparser = argparse.ArgumentParser(
    description="Tool that reads the output of bitextor-align-documents and aligns the segments of the aligned "
                "documents")
oparser.add_argument("aligned_docs", metavar="FILE", nargs='?',
                     help="File containing the set of aliged documents encoded as base64. Format is: "
                          "url1 <tab> url2 <tab> sentences1 <tab> sentences2 <tab> tokenized1 <tab> tokenzed2")
oparser.add_argument("--hunalign", dest="hunalign_bin",
                     help="Path to the hunalign executable. If this option is not defined, the executable will "
                          "be searched in the same directory where this scritp is placed")
oparser.add_argument("-d", dest="dic", help="Bilingual dictionary used for aligning and scoring")
oparser.add_argument("-t", "--tmp-dir", dest="tmpdir", default="/tmp",
                     help="Temporary directory to be used for internal temporary files (/tmp by default)")
oparser.add_argument("--hunalign-threshold", dest="hunalignthresh", type=float, default=0.0,
                     help="Threshold which will be applied to Hunalign. All the aligned segments with score lower than "
                          "this value will not be in the result. Allowed values are between 0.0 and 1.0.")
oparser.add_argument("--print-sent-hash", dest="hashprogram", default="",
                     help="provide path for a shasum like program to print Murmurhash hashes of the output sentences")
oparser.add_argument("--paragraph-identification", action="store_true",
                     help="provide path for a shasum like program to print Murmurhash hashes of the output sentences")

options = oparser.parse_args()

if options.aligned_docs is None:
    reader_list = sys.stdin
else:
    reader_list = open(options.aligned_docs, "r")

header = next(reader_list).strip().split("\t")
src_url_idx = header.index("src_url")
trg_url_idx = header.index("trg_url")
src_text_idx = header.index("src_text")
trg_text_idx = header.index("trg_text")
src_tokenized_idx = header.index("src_tokenized")
trg_tokenized_idx = header.index("trg_tokenized")

# Print output header
sys.stdout.write("src_url\ttrg_url\tsrc_text\ttrg_text\thunalign_score")

if options.paragraph_identification:
    sys.stdout.write("\tsrc_paragraph_id\ttrg_paragraph_id")

if options.hashprogram:
    sys.stdout.write("\tsrc_deferred_hash\ttrg_deferred_hash")

sys.stdout.write('\n')

for line in reader_list:
    tmp_file1 = NamedTemporaryFile(delete=False, dir=options.tmpdir)
    tmp_file2 = NamedTemporaryFile(delete=False, dir=options.tmpdir)
    tmp_file1_origtext = NamedTemporaryFile(delete=False, dir=options.tmpdir)
    tmp_file2_origtext = NamedTemporaryFile(delete=False, dir=options.tmpdir)

    fields = line.split("\t")
    fields[-1] = fields[-1].rstrip('\n')
    doc_id_1 = fields[src_url_idx]
    doc_id_2 = fields[trg_url_idx]
    encodedtext1 = fields[src_text_idx]
    encodedtext2 = fields[trg_text_idx]
    encodedtokenized1 = fields[src_tokenized_idx]
    encodedtokenized2 = fields[trg_tokenized_idx]

    tmp_file1_origtext.write(base64.b64decode(encodedtext1))
    tmp_file2_origtext.write(base64.b64decode(encodedtext2))

    tmp_file1.write(base64.b64decode(encodedtokenized1))
    tmp_file2.write(base64.b64decode(encodedtokenized2))

    tmp_file1_name = tmp_file1.name
    tmp_file2_name = tmp_file2.name
    tmp_file1_orig_name = tmp_file1_origtext.name
    tmp_file2_orig_name = tmp_file2_origtext.name

    tmp_file1.close()
    tmp_file1_origtext.close()
    tmp_file2.close()
    tmp_file2_origtext.close()

    align(doc_id_1, doc_id_2, tmp_file1_name, tmp_file2_name, tmp_file1_orig_name, tmp_file2_orig_name,
          options.dic, options.hashprogram, options.paragraph_identification, threshold=options.hunalignthresh)

    os.remove(tmp_file1.name)
    os.remove(tmp_file1_origtext.name)
    os.remove(tmp_file2.name)
    os.remove(tmp_file2_origtext.name)
