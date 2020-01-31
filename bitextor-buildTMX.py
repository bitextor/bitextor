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
# 1. The tool takes the output of bitextor-cleanalignments and formats it in TMX format
# 2. Option -c allows to define what is expected to find in each field of the input, which makes this script flexible
# about the expected fields.

# Default input format:
# url1    url2    seg1    seg2    [hunalign    bicleaner    lengthratio    numTokensSL    numTokensTL]

# where url1 and url2 are the URLs of the document, seg1 and seg2 are the aligned pair of segments, hunalign and
# bicleaner are quality metrics (in this case, provided by these two tools), lengthratio is the ratio between the
# word-length of seg1 and seg2, numTokensSL and numTokensTL is the number of tokens in each segment and is the value
# to be assigned to each TU id parameter.
#

import sys
import argparse
import time
import locale
import re
import lzma
from xml.sax.saxutils import escape


def printseg(lang, columns, urls, seg, fields_dict, mint, deferred=None, checksum=None, no_delete_seg=False):
    info_tag = []
    print("    <tuv xml:lang=\"" + lang + "\">")
    if "url1" in columns:
        for url in urls:
            print("     <prop type=\"source-document\">" + escape(url) + "</prop>")
    if deferred:
        print("     <prop type=\"deferred-seg\">" + deferred + "</prop>")
    if checksum:
        print("     <prop type=\"checksum-seg\">" + checksum + "</prop>")

    if no_delete_seg or deferred is None:
        print("     <seg>" + escape(seg) + "</seg>")
    else:
        print("     <seg></seg>")
    if "numTokensSL" in fields_dict and fields_dict["numTokensSL"] != "" and int(fields_dict["numTokensSL"]) < int(mint):
        info_tag.append("very short segments, shorter than " + str(options.mint))
    if len(info_tag) > 0:
        print("    <prop type=\"info\">" + "|".join(info_tag) + "</prop>")
    print("    </tuv>")


def printtu(idcounter, lang1, lang2, columns, urls1, urls2, fields_dict, mint, no_delete_seg):
    print("   <tu tuid=\"" + str(idcounter) + "\" datatype=\"Text\">")
    infoTag = []
    if 'hunalign' in fields_dict and fields_dict['hunalign'] != "":
        print("    <prop type=\"score-aligner\">" + fields_dict['hunalign'] + "</prop>")
    if 'bicleaner' in fields_dict and fields_dict['bicleaner'] != "":
        print("    <prop type=\"score-bicleaner\">" + fields_dict['bicleaner'] + "</prop>")
    # Output info data ILSP-FC specification
    if re.sub("[^0-9]", "", fields_dict["seg1"]) != re.sub("[^0-9]", "", fields_dict["seg2"]):
        infoTag.append("different numbers in TUVs")
    print("    <prop type=\"type\">1:1</prop>")
    if re.sub(r'\W+', '', fields_dict["seg1"]) == re.sub(r'\W+', '', fields_dict["seg2"]):
        infoTag.append("equal TUVs")
    if len(infoTag) > 0:
        print("    <prop type=\"info\">" + "|".join(infoTag) + "</prop>")

    if 'deferredseg1' not in fields_dict or fields_dict['deferredseg1'] == "":
        fields_dict['deferredseg1'] = None
    if 'deferredseg2' not in fields_dict or fields_dict['deferredseg2'] == "":
        fields_dict['deferredseg2'] = None
    if 'checksum1' not in fields_dict:
        fields_dict['checksum1'] = None
    if 'checksum2' not in fields_dict:
        fields_dict['checksum2'] = None

    printseg(lang1, columns, urls1, fields_dict['seg1'], fields_dict, mint, fields_dict['deferredseg1'],
             fields_dict['checksum1'], no_delete_seg)
    printseg(lang2, columns, urls2, fields_dict['seg2'], fields_dict, mint, fields_dict['deferredseg2'],
             fields_dict['checksum2'], no_delete_seg)

    print("   </tu>")

oparser = argparse.ArgumentParser(
    description="This script reads the output of bitextor-cleantextalign and formats the aligned segments as a TMX "
                "translation memory.")
oparser.add_argument('clean_alignments', metavar='FILE', nargs='?',
                     help='File containing the segment pairs produced by bitextor-cleantextalign (if undefined, '
                          'the script will read from standard input)',
                     default=None)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1",
                     required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2",
                     required=True)
oparser.add_argument("-q", "--min-length", help="Minimum length ratio between two parts of TU", type=float, dest="minl",
                     default=0.6)
oparser.add_argument("-m", "--max-length", help="Maximum length ratio between two parts of TU", type=float, dest="maxl",
                     default=1.6)
oparser.add_argument("-t", "--min-tokens", help="Minimum number of tokens in a TU", type=int, dest="mint", default=3)
oparser.add_argument("-c", "--columns",
                     help="Column names of the input tab separated file. Default: url1,url2,seg1,seg2. Other "
                          "options:hunalign,bifixerhash,bifixerscore,bicleaner,lengthratio,numTokensSL,numTokensTL,deferredseg1,"
                          "deferredseg2,checksum1,checksum2",
                     default="url1,url2,seg1,seg2")
oparser.add_argument("-d", "--no-delete-seg", help="Avoid deleting <seg> if deferred annotation is given",
                     dest="no_delete_seg", action='store_true')
oparser.add_argument("-f", "--text-file-deduped", help="Filename to write the deduped input file",
                     dest="text_file_deduped")
oparser.add_argument("--dedup", dest="dedup", help="Dedup entries and group urls using given columns. Like 'bifixerhash', 'seg1,seg2' , 'checksum1,checksum2'")

options = oparser.parse_args()

if options.clean_alignments is not None:
    reader = open(options.clean_alignments, "r")
else:
    reader = sys.stdin

text_writer = None
if options.text_file_deduped and options.dedup:
    if options.text_file_deduped[-3:] == ".xz":
        text_writer = lzma.open(options.text_file_deduped, "wt")
    else:
        text_writer = open(options.text_file_deduped, "w")

print("<?xml version=\"1.0\"?>")
print("<tmx version=\"1.4\">")
print(" <header")
print("   adminlang=\"" + locale.setlocale(locale.LC_ALL, '').split(".")[0].split("_")[0] + "\"")
print("   srclang=\"" + options.lang1 + "\"")
print("   o-tmf=\"PlainText\"")
print("   creationtool=\"bitextor\"")
print("   creationtoolversion=\"4.0\"")
print("   datatype=\"PlainText\"")
print("   segtype=\"sentence\"")
print("   creationdate=\"" + time.strftime("%Y%m%dT%H%M%S") + "\"")
print("   o-encoding=\"utf-8\">")
print(" </header>")
print(" <body>")

idcounter = 0 
prev_hash = ""
prev_fieldsdict = dict()
urls1 = set()
urls2 = set()
columns = options.columns.split(',')
fieldsdict = dict()
    
for line in reader:
    fields = line.split("\t")
    fields[-1] = fields[-1].strip()
    line_hash = ""
    for field, column in zip(fields, columns):
        fieldsdict[column] = field

    if options.dedup:
        for part in options.dedup.split(','):
            line_hash = line_hash + "\t" + fieldsdict[part]
    if 'seg1' not in fieldsdict:
        fieldsdict['seg1'] = ""
    if 'seg2' not in fieldsdict:
        fieldsdict['seg2'] = ""

    if (prev_hash == line_hash or prev_hash == "") and options.dedup:
        urls1.add(fieldsdict['url1'])
        urls2.add(fieldsdict['url2'])
        prev_hash = line_hash
        prev_fieldsdict = dict(fieldsdict)
    elif not options.dedup:
        urls1.add(fieldsdict['url1'])
        urls2.add(fieldsdict['url2'])
        idcounter += 1
        printtu(idcounter, options.lang1, options.lang2, columns, urls1, urls2, fieldsdict, options.mint,
                options.no_delete_seg)
        urls1 = set()
        urls2 = set()
    else:
        idcounter += 1
        printtu(idcounter, options.lang1, options.lang2, columns, urls1, urls2, prev_fieldsdict, options.mint, options.no_delete_seg)
        if text_writer:
            text_writer.write("\t".join([x for x in prev_fieldsdict.values() if x])+"\n")
        urls1 = set()
        urls2 = set()
        urls1.add(fieldsdict['url1'])
        urls2.add(fieldsdict['url2'])
        prev_hash = line_hash
        prev_fieldsdict = dict(fieldsdict)


if options.dedup:
    idcounter += 1
    if fieldsdict != {}:
        printtu(idcounter, options.lang1, options.lang2, columns, urls1, urls2, fieldsdict, options.mint, options.no_delete_seg)
    text_writer.write("\t".join([x for x in fieldsdict.values() if x])+"\n")
print(" </body>")
print("</tmx>")
reader.close()
