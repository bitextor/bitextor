#!/usr/bin/env python3

#
# 1. The tool reads as an imput the output of Bitextor and formats it in TMX format
#
# Input format:
# uri1    uri2    text1    text2
#

import sys
import argparse
import time
import locale
import re
from xml.sax.saxutils import escape


def print_tu(fieldsdict, urls1, urls2, idnum):
    print("   <tu tuid=\"" + str(idnum) + "\" datatype=\"Text\">")
    info_tag = []
    if 'hunalign' in fieldsdict and fieldsdict['hunalign'] != "":
        print("    <prop type=\"score-aligner\">" + fieldsdict['hunalign'] + "</prop>")
    if 'bicleaner' in fieldsdict and fieldsdict['bicleaner'] != "":
        print("    <prop type=\"score-bicleaner\">" + fieldsdict['bicleaner'] + "</prop>")
    # Output info data ILSP-FC specification
    if 'lengthratio' in fieldsdict and fieldsdict['lengthratio'] != "":
        print("    <prop type=\"lengthRatio\">" + str(fieldsdict['lengthratio']) + "</prop>")
    if re.sub("[^0-9]", "", fieldsdict["seg1"]) != re.sub("[^0-9]", "", fieldsdict["seg2"]):
        info_tag.append("different numbers in TUVs")
    print("    <prop type=\"type\">1:1</prop>")
    if re.sub(r'\W+', '', fieldsdict["seg1"]) == re.sub(r'\W+', '', fieldsdict["seg2"]):
        info_tag.append("equal TUVs")
    if len(info_tag) > 0:
        print("    <prop type=\"info\">" + "|".join(info_tag) + "</prop>")

    info_tag_sl = []
    info_tag_tl = []
    print("    <tuv xml:lang=\"" + options.lang1 + "\">")
    for url in urls1:
        print("     <prop type=\"source-document\">" + url.replace("&", "&amp;").replace(
            "<", "&lt;").replace(">", "&gt;").replace(
            "\"", "&quot;").replace("'", "&apos;") + "</prop>")
    print("     <seg>" + fieldsdict['seg1'].replace("&", "&amp;").replace("<", "&lt;").replace(
        ">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") + "</seg>")
    if "numTokensSL" in fieldsdict and fieldsdict["numTokensSL"] != "" and int(fieldsdict["numTokensSL"]) < int(
            options.mint):
        info_tag_sl.append("very short segments, shorter than " + str(options.mint))
    if len(info_tag_sl) > 0:
        print("    <prop type=\"info\">" + "|".join(info_tag_sl) + "</prop>")
    print("    </tuv>")
    print("    <tuv xml:lang=\"" + options.lang2 + "\">")
    for url in urls2:
        print("     <prop type=\"source-document\">" + url.replace("&", "&amp;").replace(
            "<", "&lt;").replace(">", "&gt;").replace(
            "\"", "&quot;").replace("'", "&apos;") + "</prop>")
    print("     <seg>" + fieldsdict["seg2"].replace("&", "&amp;").replace("<", "&lt;").replace(
        ">", "&gt;").replace("\"", "&quot;").replace(
        "'", "&apos;") + "</seg>")
    if "numTokensTL" in fieldsdict and fieldsdict["numTokensTL"] != "" and int(fieldsdict["numTokensTL"]) < int(
            options.mint):
        info_tag_tl.append("very short segments, shorter than " + str(options.mint))
    if len(info_tag_tl) > 0:
        print("    <prop type=\"info\">" + "|".join(info_tag_tl) + "</prop>")
    print("    </tuv>")
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
                     help="Column names of the input tab separated file. Default: url1,url2,seg1,seg2,hunalign,"
                          "bicleaner,lengthratio,numTokensSL,numTokensTL",
                     default="url1,url2,seg1,seg2,hunalign,bicleaner,lengthratio,numTokensSL,numTokensTL")
options = oparser.parse_args()

columns = options.columns.split(',')

if options.clean_alignments is not None:
    reader = open(options.clean_alignments, "r")
else:
    reader = sys.stdin
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

prevfields = reader.readline().split("\t")
prevfields[-1] = prevfields[-1].strip()

fieldsdict = dict()
for field, column in zip(prevfields, columns):
    fieldsdict[column] = field

prevfieldsdict = fieldsdict
if len(fieldsdict) > 4:
    previd = fieldsdict['seg1'] + "\t" + fieldsdict['seg2']
    urls1 = set()
    urls2 = set()
    urls1.add(fieldsdict['url1'])
    urls2.add(fieldsdict['url2'])
    idnum = 0
    for line in reader:
        fields = line.split("\t")
        fields[-1] = fields[-1].strip()

        fieldsdict = dict()
        for field, column in zip(fields, columns):
            fieldsdict[column] = field

        curid = fieldsdict['seg1'] + "\t" + fieldsdict['seg2']

        # if a new segment pair is found:
        if curid != previd:
            idnum += 1
            print_tu(prevfieldsdict, urls1, urls2, idnum)
            prevfieldsdict = fieldsdict
            urls1 = set()
            urls2 = set()
        urls1.add(fieldsdict['url1'])
        urls2.add(fieldsdict['url2'])

        previd = curid

    print_tu(fieldsdict, urls1, urls2, idnum + 1)
print(" </body>")
print("</tmx>")
reader.close()
