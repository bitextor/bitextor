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

import sys
import os
import argparse
import base64
import subprocess
from tempfile import NamedTemporaryFile


def run_aligner(filename_s, filename_t, dic, hunalign_bin, thresh="0"):
    # option -ppthresh=10?
    if dic is None or dic == "":
        if hunalign_bin is None:
            hunalign = [os.path.dirname(os.path.abspath(__file__)) + "hunalign", "-realign", "-thresh=" + thresh, "/dev/null",
                        filename_s, filename_t]
        else:
            hunalign = [hunalign_bin, "-realign", "-thresh=" + thresh, "/dev/null", filename_s, filename_t]
    else:
        if hunalign_bin is None:
            hunalign = [os.path.dirname(os.path.abspath(__file__)) + "hunalign", "-thresh=" + thresh, dic, filename_s, filename_t]
        else:
            hunalign = [hunalign_bin, "-thresh=" + thresh, dic, filename_s, filename_t]

    p = subprocess.Popen(hunalign, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    for line_o in p.stdout:
        yield line_o
    return


def align(file1, file2, file1orig, file2orig, dic, hashprogram):
    filereader1 = open(file1orig, "r")
    filereader2 = open(file2orig, "r")
    thresh = options.hunalignthresh

    if thresh is None:
        thresh = 0
    else:
        thresh = min(max(int(float(thresh) * 100.0), 0), 100)

    thresh = str(thresh)

    hunalign_output = run_aligner(file1, file2, dic, options.hunalign_bin, thresh)
    try:
        prev_hun = next(hunalign_output).strip()
        prev_fields = prev_hun.split(b"\t")
        if int(prev_fields[0]) > 0:
            for i in range(int(prev_fields[0])):
                line1 = filereader1.readline().strip()

        if int(prev_fields[1]) > 0:
            for i in range(int(prev_fields[1])):
                line2 = filereader2.readline().strip()

    except StopIteration:
        prev_hun = ""
    for line_h in hunalign_output:
        hun_line = line_h.strip()
        last_position1 = filereader1.tell()
        last_position2 = filereader2.tell()
        line1 = filereader1.readline().strip()
        line2 = filereader2.readline().strip()
        if hashprogram:
            hash1 = subprocess.run([hashprogram], stdout=subprocess.PIPE, input=line1, encoding='utf8').stdout.rstrip('\n')
            hash2 = subprocess.run([hashprogram], stdout=subprocess.PIPE, input=line2, encoding='utf8').stdout.rstrip('\n')

        prev_fields = prev_hun.split(b"\t")
        hunalign_fields = hun_line.split(b"\t")

        if float(prev_fields[2]) == -0.3:
            if int(hunalign_fields[0]) == int(prev_fields[0]):
                line1 = ""
                hash1 = ""
                filereader1.seek(last_position1)
            elif int(hunalign_fields[1]) == int(prev_fields[1]):
                line2 = ""
                hash2 = ""
                filereader2.seek(last_position2)

        if int(hunalign_fields[0]) - int(prev_fields[0]) > 1:
            for i in range((int(hunalign_fields[0]) - int(prev_fields[0])) - 1):
                tmp = filereader1.readline().strip()
                line1 += " " + tmp
                if hashprogram:
                    hash1 += "+" + subprocess.run([hashprogram], stdout=subprocess.PIPE, input=tmp, encoding='utf8').stdout.rstrip('\n')

        if int(hunalign_fields[1]) - int(prev_fields[1]) > 1:
            for i in range((int(hunalign_fields[1]) - int(prev_fields[1])) - 1):
                tmp = filereader2.readline().strip()
                line2 += " " + tmp
                if hashprogram:
                    hash2 += "+" + subprocess.run([hashprogram], stdout=subprocess.PIPE, input=tmp, encoding='utf8').stdout.rstrip('\n')
        if not hashprogram:
            print("{0}\t{1}\t{2}\t{3}\t{4}".format(filename1, filename2, line1, line2, prev_fields[2].decode("utf8")))
        else:
            print("{0}\t{1}\t{2}\t{3}\t{4}\t{5}\t{6}".format(filename1, filename2, line1, line2, prev_fields[2].decode("utf8"), hash1, hash2))

        prev_hun = hun_line

    filereader1.close()
    filereader2.close()


oparser = argparse.ArgumentParser(
    description="Tool that reads the output of bitextor-align-documents and aligns the segments of the aligned "
                "documents")
oparser.add_argument('aligned_docs', metavar='FILE', nargs='?',
                     help='File containing the set of aliged documents encoded as base64. Format is: '
                          '\'url1 <tab> url2 <tab> sentences1 <tab> sentences2 <tab> tokenized1 <tab> tokenzied2\'',
                     default=None)
oparser.add_argument("--hunalign",
                     help="Path to the hunalign executable. If this option is not defined, the executable will "
                          "be searched in the same directory where this scritp is placed",
                     dest="hunalign_bin", required=False, default=None)
oparser.add_argument("-d", help="Bilingual dictionary used for aligning and scoring", dest="dic", required=False,
                     default=None)
oparser.add_argument("-t", "--tmp-dir",
                     help="Temporary directory to be used for internal temporary files (/tmp by default)",
                     dest="tmpdir", required=False, default="/tmp")
oparser.add_argument("--hunalign-threshold",
                     help="Threshold which will be applied to Hunalign. All the aligned segments with score lower than "
                          "this value will not be in the result. Allowed values are between 0.0 and 1.0.",
                     dest="hunalignthresh", required=False, default=None)
oparser.add_argument("--print-sent-hash", help="provide path for a shasum like program to print Murmurhash hashes of the output sentences", dest="hashprogram",
                     required=False, default="")

options = oparser.parse_args()

if options.aligned_docs is None:
    reader_list = sys.stdin
else:
    reader_list = open(options.aligned_docs, "r")

for line in reader_list:
    tmp_file1 = NamedTemporaryFile(delete=False, dir=options.tmpdir)
    tmp_file2 = NamedTemporaryFile(delete=False, dir=options.tmpdir)
    tmp_file1_origtext = NamedTemporaryFile(delete=False, dir=options.tmpdir)
    tmp_file2_origtext = NamedTemporaryFile(delete=False, dir=options.tmpdir)

    fields = line.split("\t")
    filename1 = fields[0]
    filename2 = fields[1]
    encodedtext1 = fields[2]
    encodedtext2 = fields[3]
    encodedtokenized1 = fields[4]
    encodedtokenized2 = fields[5]

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

    align(tmp_file1_name, tmp_file2_name, tmp_file1_orig_name, tmp_file2_orig_name, options.dic, options.hashprogram)

    os.remove(tmp_file1.name)
    os.remove(tmp_file1_origtext.name)
    os.remove(tmp_file2.name)
    os.remove(tmp_file2_origtext.name)

