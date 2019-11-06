#!/usr/bin/env python3

import re
import sys
import os
import argparse
import base64
import string
import ast
import lzma
from toolwrapper import ToolWrapper
from external_processor import ExternalTextProcessor
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/utils")
from utils.common import open_xz_or_gzip_or_plain


def get_lang_or_default(dict, lang):
    if not dict:
        return None
    if lang in dict:
        return dict[lang]
    elif "default" in dict:
        return dict["default"]
    else:
        return None

def extract_encoded_text(encoded, proc_sent, proc_word, proc_morph, sent_writer, tok_writer):
    if not proc_sent:
        return encoded

    content = base64.b64decode(encoded).decode("utf-8").replace("\t", " ")
    tokenized_segs = proc_sent.process(content).strip()
    if tokenized_segs == None:
        return None
    else:
        tokenized_filtered = []
    
        for sent in tokenized_segs.split("\n"):
            if sum([1 for m in sent if m in string.punctuation + string.digits]) < len(sent) // 2:
                tokenized_filtered.append(sent)
        b64text = base64.b64encode("\n".join(tokenized_filtered).lower().encode("utf-8"))
        sent_writer.write("{}\n".format(b64text.decode()).encode("utf-8"))    
        if not proc_word:
            return b64text.decode()

        word_tokenized_text = []
        for line in tokenized_filtered:
            proc_word.writeline(line)
            word_tokenized_text.append(proc_word.readline())
        if not proc_morph:
            b64text = base64.b64encode("\n".join(word_tokenized_text).lower().encode("utf-8"))
            tok_writer.write("{}\n".format(b64text.decode()).encode("utf-8"))
            return b64text.decode()
 
        morph_tokenized_text = []
        if proc_morph:
            for line in word.tokenized_text:
                proc_morph.writeline(tokenized_text)
                morph_tokenized_text.append(proc_morph.readline())
    
        b64text = base64.b64encode(morph_tokenized_text.lower().encode("utf-8"))
        tok_writer.write("{}\n".format(b64text.encode("utf-8")))

        return b64text.decode()


oparser = argparse.ArgumentParser(
    description="Tool that tokenizes (sentences, tokens and morphemes) plain text")
oparser.add_argument('--folder', dest='folder', help='Bitextorlang folder', required=True)
oparser.add_argument('--langs', dest='langs', default=None,
                     help="List of  two-character language codes (comma-separated) to tokenize. "
                          "If not specified, every language will be processed")
oparser.add_argument('--sentence-splitters', dest='splitters', required=True,
                     help="A map of sentence splitter commands. "
                          "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                          "For languages that are not in this map but are in 'langs', the defaultScript will be used. "
                          "If defaultScript is not specified, language will be outputted in plain text.")
oparser.add_argument('--word-tokenizers', dest='tokenizers', required=True,
                     help="A map of word tokenisation commands. "
                          "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                          "For languages that are not in this map but are in 'langs', the defaultScript will be used. "
                          "If defaultScript is not specified, word tokenization for that language will be omitted.")
oparser.add_argument('--morph-analysers', dest='lemmatizers',
                     help="A map of morphological analysers. "
                          "Format: {\"lang1\": \"script1\", ... , \"langN\": \"scriptN\", \"default\": \"defaultScript\"}. "
                          "For languages that are not in this map but a re in 'langs', the defaultScript will be used. "
                          "If defaultScript is not specified, morphological analysis for that language will be omitted.")

options = oparser.parse_args()

if options.langs:
    langs = options.langs.split(',')
else:
    langs = []

try:
    options.splitters = ast.literal_eval(options.splitters)
except:
    print("Sentence splitters incorrect format", file=sys.stderr)
    sys.exit(1)

try:
    options.tokenizers = ast.literal_eval(options.tokenizers)
except:
    print("Word tokenizers incorrect format", file=sys.stderr)
    sys.exit(1)

try:
    if options.lemmatizers:
        options.lemmatizers = ast.literal_eval(options.lemmatizers)
except:
    print("Morphological analysers incorrect format")
    sys.exit(1)

lang_files_tok = {}
lang_files_sent = {}

folder = os.fsencode(options.folder)
for langfolder in os.listdir(folder):
    lang = os.fsdecode(langfolder)
    senttok = get_lang_or_default(options.splitters, lang)

    wordtok = get_lang_or_default(options.tokenizers, lang)
    if wordtok != None:
        wordtok_tool = ToolWrapper(wordtok.split())
    else:
        wordtok_tool = None

    morphtok = get_lang_or_default(options.lemmatizers, lang)
    if morphtok != None:
        morphtok_tool = ToolWrapper(morphtok.split())
    else:
        morphtok_tool = None

    if not os.path.isdir(options.folder+"/"+lang) or len(lang) > 2:
        continue
    fullname = os.path.join(options.folder, lang+"/plain_text.xz")
    if os.path.isfile(fullname) and (not langs or lang in langs):
        if lang not in lang_files_sent:
            lang_files_sent[lang] = lzma.open(os.path.join(options.folder, lang + "/plain_sentsplit.xz"), "wb")

        if lang not in lang_files_tok:
            lang_files_tok[lang] = lzma.open(os.path.join(options.folder, lang + "/plain_tokenized.xz"), "wb")
        with open_xz_or_gzip_or_plain(fullname) as text_reader:
            for line in text_reader:
                encodedtext = re.sub(r'(\s*[\r\n]+\s*)+', r'\r\n', line.strip())
                if senttok != None:
                    senttok_tool = ExternalTextProcessor(senttok.split())
                else:
                    senttok_tool = None

                extract_encoded_text(encodedtext, senttok_tool, wordtok_tool, morphtok_tool, lang_files_sent[lang], lang_files_tok[lang])
for lang in lang_files_tok:
    lang_files_tok[lang].close()
for lang in lang_files_sent:
    lang_files_sent[lang].close()

