#!/usr/bin/env python3


try:
    import lzma
except ImportError:
    from backports import lzma
import argparse
import base64
import gzip
import os
import string
import sys

import langid
from external_processor import ExternalTextProcessor
from textsanitzer import TextSanitizer


def filter_digits_and_punctuation(text):
    text_split = text.split()
    if len(text_split) == 1 and sum([1 for m in text_split[0] if m in string.punctuation + string.digits]) > len(text_split[0]) // 2:
        return False

    return True


def split_sentences(text, sentence_splitter_cmd, lang):
    proc = ExternalTextProcessor([sentence_splitter_cmd, "-l", lang])
    output = proc.process(text.replace("\n\n", "\n"))

    return [n for n in output.split("\n") if filter_digits_and_punctuation(n)]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--langs", dest="languages",
                        help="Languages to be extracted (comma-separated)", required=True)
    parser.add_argument("--splitter", dest="splitter",
                        help="Sentence splitting script", required=True)
    parser.add_argument("--output_prefix", dest="output_prefix", default="",
                        help="Prefix for output files within directory", required=False)
    parser.add_argument("--output_dir", dest="output_dir", default=".",
                        help="Output directory. Extracted files will be named as <lang>.extracted.gz", required=False)
    parser.add_argument("--prune", dest="prune_threshold", type=int,
                        default=80, help="Prune sentences longer than n (words/characters)", required=False)
    parser.add_argument("--prune_type", dest="prune_type", choices=set(("words", "chars")),
                        default="words", help="Prune sentences either by words or charaters", required=False)
    parser.add_argument("--check_lang", dest="check_lang", action='store_true',
                        help="Runs language identification on text segments and throws away those that do not match with the lang field", required=False)
    parser.add_argument("-x", "--xz", dest="xz", action="store_true",
                        help="Use xz as the compression tool")
    parser.add_argument('--root-dir', dest='rootDir', help='Domain directory')

    args = parser.parse_args()

    with open("{rootDir}/deduped".format(rootDir=args.rootDir), "rt") as dedupedFile:
        dedupeds = dedupedFile.read().strip().split("\n")

    with open("{rootDir}/raw-html/page".format(rootDir=args.rootDir), "rt") as pageFile:
        pages = pageFile.read().strip().split("\n")

    #sys.stderr.write("args.rootDir=" + args.rootDir + "\n")

    langs_parse = args.languages.strip().split(',')
    lang_file = {}
    for l in langs_parse:
        if not l.strip():
            continue
        if args.xz:
            lang_file[l] = lzma.open(os.path.join(
                args.output_dir, "{0}{1}.extracted.xz".format(args.output_prefix,l)), "wb")
        else:
            lang_file[l] = gzip.open(os.path.join(
                args.output_dir, "{0}{1}.extracted.gz".format(args.output_prefix,l)), "wb")

    for line in dedupeds:
        lineNum = int(line)
        #sys.stderr.write("lineNum=" + str(lineNum) + "\n")

        pageToks = pages[lineNum].split("\t")
        assert(len(pageToks) == 5)
        #sys.stderr.write("pageToks=" + str(pageToks) + "\n")

        with open("{rootDir}/text/{name}".format(rootDir=args.rootDir, name=lineNum), "rt") as textFile:
            text = textFile.read()

        lang = pageToks[4]
        uri = pageToks[0]

        if lang not in langs_parse:
            continue

        if not text.strip():
            continue

        # clean the UTF8 text
        text = TextSanitizer.clean_text(text)

        for extracted_line in split_sentences(text, args.splitter, lang):
            extracted_line = extracted_line.strip()
            if not extracted_line:
                continue

            # prune long sentences
            extracted_line = extracted_line
            if args.prune_type == "chars":
                if len(extracted_line) > args.prune_threshold:
                    continue
            elif args.prune_type == "words":
                if len(extracted_line.split()) > args.prune_threshold:
                    continue

            if args.check_lang:
                # check that the content is actually in the expected language
                detected_language = langid.classify(extracted_line)[0]
                if detected_language != lang:
                    continue

            lang_file[lang].write("{0}\t{1}\n".format(
                uri, extracted_line).encode("utf-8"))

    for f in lang_file:
        lang_file[f].close()
