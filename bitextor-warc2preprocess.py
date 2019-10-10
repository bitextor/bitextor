#!/usr/bin/env python3

import html
from warcio.archiveiterator import ArchiveIterator
import base64
import argparse
import cchardet
import magic
import re
from bs4 import BeautifulSoup
import jpype
import os
import imp
import alcazar.bodytext
import logging
import lzma
import gzip
from selectolax.parser import HTMLParser
import mmh3

if not jpype.isJVMStarted():
    jars = []
    for top, dirs, files in os.walk(imp.find_module('boilerpipe')[1] + '/data'):
        for nm in files:
            if nm[-4:] == ".jar":
                jars.append(os.path.join(top, nm))
    jpype.addClassPath(os.pathsep.join(jars))
    jpype.startJVM(jpype.getDefaultJVMPath(), convertStrings=False)
from boilerpipe.extract import Extractor as ExtrB

def guess_lang_from_data2(data):
    reliable, text_bytes, detected_languages = cld2.detect(
        ''.join(x for x in data if x.isprintable()), isPlainText=False)
    return detected_languages[0][1]

def guess_lang_from_data3(model, data):
    language, probability, reliable, proportion = model.get_language(data)
    return language


def convert_encoding(data):
    encoding = cchardet.detect(data)['encoding']
    if encoding is None:
        encoding = "utf-8"
    if len(data) > 0:
        # We convert, even if the text is detected to be UTF8 so, if it is an error and conversion fails, the error
        # is catched here
        for enc in [encoding, 'utf-8', 'iso-8859-1', 'windows‑1252']:
            try:
                return enc, data.decode(enc)
            except:
                pass
    return None, ''

oparser = argparse.ArgumentParser(
    description="Script that takes every record in a WARC file and runs preprocessing, which includes: HTML"
                "normalization, deduplication, MIME and language identification, and boilerplate removing. The result"
                "of each pre-processing step is stored in a XZ compressed file in the output directory.")
oparser.add_argument("--verbose", action="store_true", default=False,
                     help="Produce additional information about preprocessing through stderr.")
oparser.add_argument("--boilerpipe", action="store_true", default=False,
                     help="Use boilerpipe bodytext to do the de-boiling")
oparser.add_argument("--parser", dest="parser", default="bs4",
                     help="Use 'modest', 'bs4' or 'alcazar' parsers to extract relevant text from HTML. By default 'modest' is used")
oparser.add_argument('--output-dir', dest='outDir', help='Output directory', required=True)
oparser.add_argument('--output_hash', dest='outputHash', help='Output path for Murmur Hash of plain texts')
oparser.add_argument('--input_hash', dest='inputHash', help='Input path for previous Bitextor Murmur Hash plain texts file')
oparser.add_argument('--lang1', dest='l1', help='Language l1 in the crawl', default=None)
oparser.add_argument('--lang2', dest='l2', help='Language l2 in the crawl', default=None)
oparser.add_argument('--input', dest='input', help='Input WARC file', default=None)
oparser.add_argument('--xzlang', action="store_true", help='Separate output into different files by language',
                     default=False)
oparser.add_argument('--langs', dest="langs", default="",
                     help='List of languages to include or ignore (%%): l1,l2,%%l3,%%l4')
oparser.add_argument('--langid', dest="langid", default="cld2", help="Model used for language detection: cld2 or cld3")
options = oparser.parse_args()

logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO if options.verbose else logging.ERROR, datefmt='%Y-%m-%d %H:%M:%S')
if options.input[-3:] == ".xz":
    f = ArchiveIterator(lzma.open(options.input, 'r'))
elif options.input[-3:] == ".gz":
    f = ArchiveIterator(gzip.open(options.input, 'r'))
else:
    f = ArchiveIterator(open(options.input, 'r'))
seen_html = set()
seen_plain_text = set()

magic.Magic(mime=True)

languages = []
banned = []
if options.langid == "cld3":
    import cld3
    cld3model = cld3.LanguageIdentifier()
else:
    import pycld2 as cld2


if options.langs:
    for l in options.langs.split(','):
        if l[0] == '+':
            languages.append(l[1:])
        elif l[0] == '%':
            banned.append(l[1:])
        else:
            languages.append(l)

# make sure that if languages are specified, lang1 and lang2 are among them
if languages:
    if options.l1 is not None:
        languages.append(options.l1)
    if options.l2 is not None:
        languages.append(options.l2)

previous_crawl_hashes=set()

if options.inputHash:
    with lzma.open(options.inputHash,"r") as fh:
        for line in fh:
            previous_crawl_hashes.add(int(line.strip()))

if options.outputHash:
    plainTextHashFile = lzma.open(options.outputHash, "w")

files_dict = dict()

for record in f:
    # Initial checks
    if record.rec_type != 'response':
        continue
    if record.rec_headers.get_header('WARC-Target-URI')[0] == '<' and record.rec_headers.get_header('WARC-Target-URI')[-1] == '>':
        url = record.rec_headers.get_header('WARC-Target-URI')[1:-1]
    else:
        url = record.rec_headers.get_header('WARC-Target-URI')
    if url == "unknown":
        logging.info("Skipping page with unknown URL")
        continue
    url = url.lower()
    url = url.replace('\t',' ')
    if url[-4:] == ".gif" or url[-4:] == ".jpg" or url[-5:] == ".jpeg" or url[-4:] == ".png" or url[-4:] == ".css" or url[-3:] == ".js" or url[-4:] == ".mp3" or url[-4:] == ".mp4" or url[-4:] == ".ogg" or url[-5:] == ".midi" or url[-4:] == ".swf":
        continue

    # Ignore robots.txt when processing records
    if url[-11:] == "/robots.txt":
        continue
    payload = record.content_stream().read()

    date = record.rec_headers.get_header('WARC-Date')
    recordId = record.rec_headers.get_header('WARC-Record-ID')
    # We convert into UTF8 first of all
    orig_encoding, text = convert_encoding(payload)
    logging.info("Processing document: " + url)
    if orig_encoding is None:
        logging.info("Encoding of document " + url + " could not be identified")

    if len(text) > 0:
        # lang id
        logging.info(url + ": detecting language")
        lang = ""
        if (options.langid == "cld3"):
            lang = guess_lang_from_data3(cld3model, text)
        else:
            lang = guess_lang_from_data2(text)
        if (len(languages) > 0 and lang not in languages) or (lang in banned):
            logging.info("Language of document " + url + ": " + lang + ". Not among searched languages.")
        else:
            if not options.xzlang:
                if lang not in files_dict:
                    if not os.path.exists(options.outDir + "/" + lang):
                        os.makedirs(options.outDir + "/" + lang)
                    urlFile = lzma.open(options.outDir + "/" + lang + "/url.xz", "w")
                    encodingFile = lzma.open(options.outDir + "/" + lang + "/encoding.xz", "w")
                    mimeFile = lzma.open(options.outDir + "/" + lang + "/mime.xz", "w")
                    normHtmlFile = lzma.open(options.outDir + "/" + lang + "/normalized_html.xz", "w")
                    plainTextFile = lzma.open(options.outDir + "/" + lang + "/plain_text.xz", "w")
                    if options.boilerpipe:
                        deboilFile = lzma.open(options.outDir + "/" + lang + "/" + "deboilerplate_html.xz", "w")
                        files_dict[lang] = {"urlFile": urlFile, "encodingFile": encodingFile, "mimeFile": mimeFile, "normHtmlFile": normHtmlFile, "plainTextFile": plainTextFile, "deboilFile": deboilFile}
                    else:
                        if not os.path.exists(options.outDir + "/" + lang + "/" + "deboilerplate_html.xz")\
                                and not os.path.islink(options.outDir + "/" + lang + "/" + "deboilerplate_html.xz"):
                            os.symlink("normalized_html.xz", options.outDir + "/" + lang + "/" + "deboilerplate_html.xz")
                        files_dict[lang] = {"urlFile": urlFile, "encodingFile": encodingFile, "mimeFile": mimeFile, "normHtmlFile": normHtmlFile, "plainTextFile": plainTextFile}

            # If enabled, remove boilerplate HTML
            if options.boilerpipe:
                logging.info(url + ": deboiling html")
                extractor = ExtrB(extractor='ArticleExtractor', html=text)
                deboiled = str(extractor.getHTML())
            else:
                deboiled = text

            # We compute a hash on the HTML (either normalized one or after boilerpipe if enabled):
            # if we get duplicate files we discard them
            html_hash=mmh3.hash(deboiled, signed=False)
            # print("hash", c.hexdigest(), url)
            # checking for duplicate content (duplicates are discarded)
            if html_hash in seen_html:
                logging.info("Repeated file:\t" + url)
                continue
                # get text with Alcazar library
            if options.parser == "alcazar":
                logging.info(url + ": Getting text with Alcazar")
                btext = alcazar.bodytext.parse_article(deboiled)
                if btext.body_text:
                    plaintext = btext.body_text
                else:
                    plaintext = ""
            # or get text with beautifulsoup
            elif options.parser == "bs4":
                logging.info(url + ": Getting text with BeautifulSoup")
                soup = BeautifulSoup(deboiled, "lxml")
                for script in soup(["script", "style", "img"]):
                    script.extract()  # rip it out
                plaintext = soup.get_text()
            # or get text with 'modest' library
            else:
                logging.info(url + ": Getting text with modest (selectolax)")
                try:
                    tree = HTMLParser(deboiled)
                except:
                    logging.info("Tree structure issues in HTML/XML. Ignoring this document")
                    continue
                for tag in tree.css('script'):
                    tag.decompose()
                for tag in tree.css('style'):
                    tag.decompose()
                for tag in tree.css('img'):
                    tag.decompose()
                if tree.body is None:
                    logging.info("Body is empty. Ignoring this document")
                    continue
                plaintext = tree.body.text(separator='\n')
                plaintext = re.sub(r"\n+", "\n",re.sub(r" *\n *", "\n", re.sub(r" +", " ", re.sub(r"\r", "", plaintext))))

                plaintext_hash=mmh3.hash(plaintext,signed =False)

                if plaintext_hash in seen_plain_text or plaintext_hash in previous_crawl_hashes:
                    logging.info("Repeated plain text file:\t" + url)
                    continue

                if len(plaintext) > 0:
                    seen_html.add(html_hash)
                    seen_plain_text.add(plaintext_hash)
                    # Guessing MIME of the file (checked on original content)
                    logging.info(url + ": Getting mime")
                    mime = magic.from_buffer(text, mime=True)

                    if not options.xzlang:
                        files_dict[lang]["mimeFile"].write(mime.encode() + b"\n")

                        files_dict[lang]["urlFile"].write(url.encode() + b"\n")
                        files_dict[lang]["encodingFile"].write(orig_encoding.encode() + b"\n")

                        b64norm = base64.b64encode(text.encode())
                        files_dict[lang]["normHtmlFile"].write(b64norm + b"\n")

                        if options.boilerpipe:
                            b64deboil = base64.b64encode(deboiled.encode())
                            files_dict[lang]["deboilFile"].write(b64deboil + b"\n")

                        b64text = base64.b64encode(html.unescape(plaintext).encode())
                        files_dict[lang]["plainTextFile"].write(b64text + b"\n")
                    # append to language specific file
                    else:
                        langfile = lzma.open(options.outDir + "/" + lang, mode="a", format=lzma.FORMAT_XZ)
                        header = "Content-Location: " + url + "\n"
                        header += "Content-Type: " + mime + "\n"
                        header += "Content-Language: " + lang + "\n"
                        header += "Content-Length: " + str(len(plaintext)) + "\n"
                        header += "Date: " + date + "\n"
                        header += "X-WARC-Record-Id: " + recordId + "\n"
                        header += "X-WARC-Filename: " + options.input + "\n"
                        langfile.write(header.encode())
                        langfile.write(b"\n")
                        langfile.write(plaintext.encode())
                        langfile.write(b"\n")
                        langfile.close()

                    if options.outputHash:
                        plainTextHashFile.write(str(plaintext_hash).encode() + b"\n")

if not options.xzlang:
    for lang in files_dict:
        files_dict[lang]["urlFile"].close()
        files_dict[lang]["encodingFile"].close()
        files_dict[lang]["mimeFile"].close()
        files_dict[lang]["normHtmlFile"].close()
        files_dict[lang]["plainTextFile"].close()
        if options.boilerpipe:
            files_dict[lang]["deboilFile"].close()
    if options.outputHash:
        plainTextHashFile.close()

