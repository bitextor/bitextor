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
import html
import ast

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../utils")
from external_processor import ExternalTextProcessor
from common import open_xz_or_gzip_or_plain

def get_name(output_dir, prefix, language, tokenized, xz):
    if tokenized:
        basename = "{0}{1}.extracted.tokenized".format(prefix, language)
    else:
        basename = "{0}{1}.extracted".format(prefix, language)

    if xz:
        return os.path.join(output_dir, basename+".xz")
    else:
        return os.path.join(output_dir, basename+".gz")

def filter_digits_and_punctuation(original_text):
    text_split = original_text.split()
    if len(text_split) == 1 and sum([1 for m in text_split[0] if m in string.punctuation + string.digits]) > len(
            text_split[0]) // 2:
        return False

    return True


def split_sentences(original_text, sentence_splitter_cmd):
    if sentence_splitter_cmd:
        proc = ExternalTextProcessor(sentence_splitter_cmd.split())
        text_split = proc.process(original_text.replace("\n\n", "\n"))
    else:
        text_split = original_text.replace("\n\n", "\n")

    output = html.unescape(text_split)

    return [n for n in output.split("\n") if filter_digits_and_punctuation(n)]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--lang1", dest="l1", default="", help="Source language")
    parser.add_argument("--lang2", dest="l2", default="", help="Target language")
    parser.add_argument("--langs", dest="languages", default="",
                        help="Languages to be extracted (comma-separated). "
                             "If non are specified, all the languages found will be extracted.")
    parser.add_argument("--splitters", dest="splitters", default="",
                        help="A map of sentence splitter commands. "
                             "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                             "For languages that are not in this map but are in 'langs', the defaultScript will be used. "
                             "If defaultScript is not specified, language will be omitted.")
    parser.add_argument("--tokenized", dest="tokenized", action="store_true",
                        help='Don\'t apply sentence splitter to the text (split by newlines only). '
                             'Output files will be named <lang>.extracted.tokenized.gz')
    parser.add_argument("--output_prefix", dest="output_prefix", default="",
                        help="Prefix for output files within directory", required=False)
    parser.add_argument("--output_dir", dest="output_dir", default=".",
                        help="Output directory. Extracted files will be named as <lang>.extracted.gz", required=False)
    parser.add_argument("--prune", dest="prune_threshold", type=int,
                        default=80, help="Prune sentences longer than n (words/characters)", required=False)
    parser.add_argument("--prune_type", dest="prune_type", choices={"words", "chars"},
                        default="words", help="Prune sentences either by words or charaters", required=False)
    parser.add_argument("-x", "--xz", dest="xz", action="store_true",
                        help="Use xz as the compression tool")
    parser.add_argument('--bitextorlang', dest='bitextorlang', help='Bitextorlang folder')

    args = parser.parse_args()

    if not args.tokenized:
        try:
            args.splitters = ast.literal_eval(args.splitters)
        except:
            print("Sentence splitters incorrect format", file=sys.stderr)
            sys.exit(1)

    if args.languages != "":
        langs_parse = args.languages.strip().split(',')
    else:
        langs_parse = []

    # make sure that if languages are specified, lang1 and lang2 are among them
    if langs_parse:
        if args.l1 and args.l1 not in langs_parse:
            langs_parse.append(args.l1)
        if args.l2 and args.l2 not in langs_parse:
            langs_parse.append(args.l2)

    lang_file = {}
    if args.tokenized:
        text_file_name = "/plain_tokenized.xz"
    else:
        text_file_name = "/plain_text.xz"
    for lang in langs_parse:
        if not lang.strip():
            continue

        lang_file[lang] = lzma.open(get_name(args.output_dir, args.output_prefix, lang, args.tokenized, args.xz), "wb")

        with open_xz_or_gzip_or_plain(args.bitextorlang+"/"+lang+text_file_name) as text_reader, \
                open_xz_or_gzip_or_plain(args.bitextorlang+"/"+lang+"/url.xz") as url_reader:
            for line in text_reader:
                text = base64.b64decode(line.strip()).decode("utf-8")
                uri = next(url_reader, None).strip()

                if not text:
                    continue

                if args.tokenized:
                    split = split_sentences(text, None)
                elif lang in args.splitters:
                    split = split_sentences(text, args.splitters[lang])
                elif "default" in args.splitters:
                    split = split_sentences(text, args.splitters["default"])
                else:
                    continue

                for extracted_line in split:

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

                    if lang not in lang_file:
                        lang_file[lang] = lzma.open(get_name(args.output_dir, args.output_prefix, lang, args.tokenized, args.xz), "wb")

                    lang_file[lang].write("{0}\t{1}\n".format(uri, extracted_line).encode("utf-8"))



        # print("lang_file", lang_file)
        for f in lang_file:
            lang_file[f].close()
