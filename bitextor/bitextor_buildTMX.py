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
# src_url    trg_url    src_text    trg_text    [hunalign_score | bleualign_score    bicleaner_score | bicleaner_ai_score    length_ratio    src_num_tokens    trg_num_tokens]

# where src_url and trg_url are the URLs of the document, src_text and trg_text are the aligned pair of segments, hunalign and
# bicleaner are quality metrics (in this case, provided by these two tools), length_ratio is the ratio between the
# word-length of src_text and trg_text, src_num_tokens and trg_num_tokens is the number of tokens in each segment and is the
# value to be assigned to each TU id parameter.
#

import sys
import argparse
import time
import locale
import re
from xml.sax.saxutils import escape

from bitextor.utils.common import open_xz_or_gzip_or_plain, dummy_open


def printseg(lang, seg_columns, urls, seg, fields_dict, mint, checksum=None, no_delete_seg=False, paragraph_id=None):
    info_tag = []

    print("    <tuv xml:lang=\"" + lang + "\">")

    if "src_url" in seg_columns:
        for url in urls:
            print("     <prop type=\"source-document\">" + escape(url) + "</prop>")

    if checksum:
        print("     <prop type=\"checksum-seg\">" + checksum + "</prop>")

    if paragraph_id:
        print("     <prop type=\"paragraph-id\">" + paragraph_id + "</prop>")

    if lang == "en":
        if 'en_document_level_variant' in fields_dict and fields_dict['en_document_level_variant'] != "":
            print("    <prop type=\"english-variant-document\">" + fields_dict['en_document_level_variant'] + "</prop>")
        if 'en_domain_level_variant' in fields_dict and fields_dict['en_domain_level_variant'] != "":
            print("    <prop type=\"english-variant-domain\">" + fields_dict['en_domain_level_variant'] + "</prop>")

    if no_delete_seg or checksum is None:
        print("     <seg>" + escape(seg) + "</seg>")
    else:
        print("     <seg></seg>")

    if "src_num_tokens" in fields_dict and fields_dict["src_num_tokens"] != "" \
            and int(fields_dict["src_num_tokens"]) < int(mint):
        info_tag.append("very short segments, shorter than " + str(options.mint))

    if len(info_tag) > 0:
        print("    <prop type=\"info\">" + "|".join(info_tag) + "</prop>")

    print("    </tuv>")


def printtu(tu_idcounter, lang1, lang2, tu_columns, tu_urls1, tu_urls2, fields_dict, mint, no_delete_seg):
    info_tag = []

    print("   <tu tuid=\"" + str(tu_idcounter) + "\" datatype=\"Text\">")

    if 'hunalign_score' in fields_dict and fields_dict['hunalign_score'] != "":
        print("    <prop type=\"score-hunalign\">" + fields_dict['hunalign_score'] + "</prop>")
    elif 'bleualign_score' in fields_dict and fields_dict['bleualign_score'] != "":
        print("    <prop type=\"score-bleualign\">" + fields_dict['bleualign_score'] + "</prop>")

    if 'bicleaner_score' in fields_dict and fields_dict['bicleaner_score'] != "":
        print("    <prop type=\"score-bicleaner\">" + fields_dict['bicleaner_score'] + "</prop>")
    elif 'bicleaner_ai_score' in fields_dict and fields_dict['bicleaner_ai_score'] != "":
        print("    <prop type=\"score-bicleaner-ai\">" + fields_dict['bicleaner_ai_score'] + "</prop>")
    if 'biroamer_entities_detected' in fields_dict and fields_dict['biroamer_entities_detected'] != "":
        print("    <prop type=\"biroamer-entities\">" + fields_dict['biroamer_entities_detected'] + "</prop>")

    # Output info data ILSP-FC specification
    if re.sub("[^0-9]", "", fields_dict["src_text"]) != re.sub("[^0-9]", "", fields_dict["trg_text"]):
        info_tag.append("different numbers in TUVs")

    print("    <prop type=\"type\">1:1</prop>")

    if re.sub(r'\W+', '', fields_dict["src_text"]) == re.sub(r'\W+', '', fields_dict["trg_text"]):
        info_tag.append("equal TUVs")

    if len(info_tag) > 0:
        print("    <prop type=\"info\">" + "|".join(info_tag) + "</prop>")

    # Initialize fields if value was not provided
    for field in ('src_deferred_hash', 'trg_deferred_hash', 'src_paragraph_id', 'trg_paragraph_id'):
        if field not in fields_dict:
            fields_dict[field] = None

    printseg(
        lang1,
        tu_columns,
        tu_urls1,
        fields_dict['src_text'],
        fields_dict,
        mint,
        fields_dict['src_deferred_hash'],
        no_delete_seg,
        fields_dict['src_paragraph_id'])
    printseg(
        lang2,
        tu_columns,
        tu_urls2,
        fields_dict['trg_text'],
        fields_dict,
        mint,
        fields_dict['trg_deferred_hash'],
        no_delete_seg,
        fields_dict['trg_paragraph_id'])

    print("   </tu>")


oparser = argparse.ArgumentParser(
    description="This script reads the output of bitextor-cleantextalign and formats the aligned segments as a TMX "
                "translation memory.")

oparser.add_argument('clean_alignments', metavar='FILE', nargs='?', default=None,
                     help="File containing the segment pairs produced by bitextor-cleantextalign (if undefined, "
                          "the script will read from standard input)")

oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1",
                     required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2",
                     required=True)
oparser.add_argument("-q", "--min-length", help="Minimum length ratio between two parts of TU", type=float, dest="minl",
                     default=0.6)
oparser.add_argument("-m", "--max-length", help="Maximum length ratio between two parts of TU", type=float, dest="maxl",
                     default=1.6)
oparser.add_argument("-t", "--min-tokens", help="Minimum number of tokens in a TU", type=int, dest="mint", default=3)
oparser.add_argument("-d", "--no-delete-seg", help="Avoid deleting <seg> if standoff annotation checksum is given",
                     dest="no_delete_seg", action='store_true')
oparser.add_argument("-f", "--text-file-deduped", help="Filename to write the deduped input file",
                     dest="text_file_deduped")
oparser.add_argument("--dedup", dest="dedup", help="Dedup entries and group urls using given columns. "
                     "Like 'bifixer_hash', 'src_text,trg_text' , 'src_deferred_hash,trg_deferred_hash'")

options = oparser.parse_args()

with open_xz_or_gzip_or_plain(options.clean_alignments, 'rt') if options.clean_alignments else sys.stdin as reader, \
     open_xz_or_gzip_or_plain(options.text_file_deduped, 'wt') if options.text_file_deduped and options.dedup else dummy_open() as text_writer:

    print("<?xml version=\"1.0\"?>")
    print("<tmx version=\"1.4\">")
    print(" <header")
    print("   adminlang=\"" + locale.setlocale(locale.LC_ALL, '').split(".")[0].split("_")[0] + "\"")
    print("   srclang=\"" + options.lang1 + "\"")
    print("   o-tmf=\"PlainText\"")
    print("   creationtool=\"bitextor\"")
    print("   creationtoolversion=\"8.2\"")
    print("   datatype=\"PlainText\"")
    print("   segtype=\"sentence\"")
    print("   creationdate=\"" + time.strftime("%Y%m%dT%H%M%S") + "\"")
    print("   o-encoding=\"utf-8\">")
    print(" </header>")
    print(" <body>")

    idcounter = 0
    prev_hash = ""
    urls1 = set()
    urls2 = set()
    bestseg = dict()
    bestchecksum1 = ""
    bestchecksum2 = ""
    header = next(reader).strip().split('\t')
    fieldsdict = dict()

    if text_writer:
        # Print output header
        text_writer.write('\t'.join(header) + '\n')

    for line in reader:
        fields = line.split("\t")
        fields[-1] = fields[-1].strip()
        line_hash = ""

        for field, column in zip(fields, header):
            fieldsdict[column] = field

        if options.dedup:
            for part in options.dedup.split(','):
                line_hash = line_hash + "\t" + fieldsdict[part]

        if 'src_text' not in fieldsdict:
            fieldsdict['src_text'] = ""

        if 'trg_text' not in fieldsdict:
            fieldsdict['trg_text'] = ""

        if prev_hash == "" and options.dedup:
            bestseg = dict(fieldsdict)
            urls1.add(fieldsdict['src_url'])
            urls2.add(fieldsdict['trg_url'])
            prev_hash = line_hash
        elif prev_hash == line_hash and options.dedup:
            urls1.add(fieldsdict['src_url'])
            urls2.add(fieldsdict['trg_url'])
            prev_hash = line_hash
        elif not options.dedup:
            urls1.add(fieldsdict['src_url'])
            urls2.add(fieldsdict['trg_url'])
            idcounter += 1
            printtu(idcounter, options.lang1, options.lang2, header, urls1, urls2, fieldsdict, options.mint,
                    options.no_delete_seg)
            urls1 = set()
            urls2 = set()
        else:
            idcounter += 1
            printtu(
                idcounter,
                options.lang1,
                options.lang2,
                header,
                urls1,
                urls2,
                bestseg,
                options.mint,
                options.no_delete_seg)

            if text_writer:
                text_writer.write("\t".join([x for x in bestseg.values() if x]) + "\n")

            urls1 = set()
            urls2 = set()
            bestseg = dict(fieldsdict)
            urls1.add(fieldsdict['src_url'])
            urls2.add(fieldsdict['trg_url'])
            prev_hash = line_hash

    if options.dedup:
        idcounter += 1

        if fieldsdict != {}:
            printtu(
                idcounter,
                options.lang1,
                options.lang2,
                header,
                urls1,
                urls2,
                fieldsdict,
                options.mint,
                options.no_delete_seg)

        if text_writer:
            text_writer.write("\t".join([x for x in fieldsdict.values() if x]) + "\n")

    print(" </body>")
    print("</tmx>")
