#!/usr/bin/env python3

import html
from warcio.archiveiterator import ArchiveIterator
import base64
import sys
import argparse
import cchardet
import hashlib
import magic
import re
import ftfy
import pycld2 as cld2
from lxml.html.clean import Cleaner
from bs4 import BeautifulSoup
import jpype
import os
import imp
import alcazar.bodytext
import logging
import lzma
import subprocess
import gzip
import zipfile
import io
from selectolax.parser import HTMLParser

if not jpype.isJVMStarted():
    jars = []
    for top, dirs, files in os.walk(imp.find_module('pdfextract')[1] + '/data'):
        for nm in files:
            if nm[-4:] == ".jar":
                jars.append(os.path.join(top, nm))
    for top, dirs, files in os.walk(imp.find_module('boilerpipe')[1] + '/data'):
        for nm in files:
            if nm[-4:] == ".jar":
                jars.append(os.path.join(top, nm))
    jpype.addClassPath(os.pathsep.join(jars))
    jpype.startJVM(jpype.getDefaultJVMPath(), convertStrings=False)
from boilerpipe.extract import Extractor as ExtrB
from pdfextract.extract import Extractor as ExtrP


def guess_lang_from_data2(data):
    reliable, text_bytes, detected_languages = cld2.detect(
        ''.join(x for x in data if x.isprintable()), isPlainText=False)
    return detected_languages[0][1]


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


def pdf2html(data):
    pconverter = subprocess.Popen(["pdftohtml", "-i", "-stdout", "-", "-"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    converter_stdout, error = pconverter.communicate(input=data)
    return [converter_stdout.replace(b"&#160;", b" ")]


def pdfextract_shell(data):
    pconverter = subprocess.Popen(["sh", "-c", "datafile=`mktemp`; cat - > $datafile.pdf; dataoutputfile=`mktemp`; java -jar pdf-extract/runnable-jar/PDFExtract.jar -I $datafile.pdf -O $dataoutputfile > /dev/null ; cat $dataoutputfile ; rm $datafile $datafile.pdf $dataoutputfile"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    converter_stdout, error = pconverter.communicate(input=data)
    return [converter_stdout]


def pdfextract(data, extractor):
    extractor.setData(data)
    return [bytes(extractor.getHTML(), 'utf8')]


def openoffice2html(data):
    datastream = io.BytesIO(data)
    try:
        openoffice_file = zipfile.ZipFile(datastream)
    except zipfile.BadZipFile:
        return []
    return [openoffice_file.read('content.xml')]


def office2html(data):
    datastream = io.BytesIO(data)
    try:
        office_file = zipfile.ZipFile(datastream)
    except zipfile.BadZipFile:
        return []
    # word/document.xml, ppt/slides/slide*.xml, xl/sharedStrings.xml
    xmls = []
    for xml in office_file.namelist():
        if "word/document.xml" == xml or "ppt/slides/slide" == xml[0:16] or "xl/sharedStrings.xml" == xml:
            xmls.append(office_file.read(xml))
    return xmls


def epub2html(data):
    datastream = io.BytesIO(data)
    try:
        epub_file = zipfile.ZipFile(datastream)
    except zipfile.BadZipFile:
        return []
    # EPUB/*html
    xmls = []
    for xml in epub_file.namelist():
        if "ml" == xml[-2:]:
            xmls.append(epub_file.read(xml))
    return xmls


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
oparser.add_argument('--prefix', dest='prefix', help='Prefix of the file name; if not specified it is empty string',
                     required=False, default="")
oparser.add_argument('--lang1', dest='l1', help='Language l1 in the crawl', default=None)
oparser.add_argument('--lang2', dest='l2', help='Language l2 in the crawl', default=None)
oparser.add_argument('--input', dest='input', help='Input WARC file', default=None)
oparser.add_argument('--pdfextract', action="store_true", help='Use pdf-extract engine or pdftohtml for PDFs',
                     default=False)
oparser.add_argument('--xzlang', action="store_true", help='Separate output into different files by language',
                     default=False)
oparser.add_argument('--langs', dest="langs", default="",
                     help='List of languages to include (+) or ignore (%%): +l1,+l2,%%l3,%%l4')
options = oparser.parse_args()

logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO if options.verbose else logging.ERROR, datefmt='%Y-%m-%d %H:%M:%S')
if options.input[-3:] == ".xz":
    f = ArchiveIterator(lzma.open(options.input, 'r'))
elif options.input[-3:] == ".gz":
    f = ArchiveIterator(gzip.open(options.input, 'r'))
else:
    f = ArchiveIterator(open(options.input, 'r'))
seen_md5 = {}
magic.Magic(mime=True)

languages = []
banned = []
if options.l1 is not None:
    languages.append(options.l1)
if options.l2 is not None:
    languages.append(options.l2)
if options.langs is not "":
    for l in options.langs.split(','):
        if l[0] == '+':
            languages.append(l[1:])
        elif l[0] == '%':
            banned.append(l[1:])

if not options.xzlang:
    urlFile = lzma.open(options.outDir + "/" + options.prefix + "url.xz", "w")
    langFile = lzma.open(options.outDir + "/" + options.prefix + "lang.xz", "w")
    encodingFile = lzma.open(options.outDir + "/" + options.prefix + "encoding.xz", "w")
    mimeFile = lzma.open(options.outDir + "/" + options.prefix + "mime.xz", "w")
    normHtmlFile = lzma.open(options.outDir + "/" + options.prefix + "normalized_html.xz", "w")
    plainTextFile = lzma.open(options.outDir + "/" + options.prefix + "plain_text.xz", "w")
# Boilerpipe cleaning is optional
if options.boilerpipe:
    deboilFile = lzma.open(options.outDir + "/" + options.prefix + "deboilerplate_html.xz", "w")
if options.pdfextract:
    extractor = ExtrP()

num = 0
cleaner = Cleaner(style=True, links=True, add_nofollow=True, page_structure=False, safe_attrs_only=False)

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
    if "text/dns" in record.rec_headers.get_header('Content-Type'):
        continue
    pageSize = int(record.rec_headers.get_header('Content-Length'))
    if pageSize > 5242880:
        logging.info("Skipping page, over limit. " + str(pageSize) + " " + url)
        continue
    if record.http_headers is not None and record.http_headers.get_header('Content-Type') is not None:
        if "image/" in record.http_headers.get_header('Content-Type') or "audio/" in record.http_headers.get_header(
                'Content-Type') or "video/" in record.http_headers.get_header(
                'Content-Type') or "text/x-component" in record.http_headers.get_header(
                'Content-Type') or "text/x-js" in record.http_headers.get_header(
                'Content-Type') or "text/javascript" in record.http_headers.get_header(
                'Content-Type') or "application/x-javascript" in record.http_headers.get_header(
                'Content-Type') or "text/css" in record.http_headers.get_header(
                'Content-Type') or "application/javascript" in record.http_headers.get_header(
                'Content-Type') or "application/x-shockwave-flash" in record.http_headers.get_header(
                'Content-Type') or "application/octet-stream" in record.http_headers.get_header(
                'Content-Type') or "application/x-font-ttf" in record.http_headers.get_header('Content-Type'):
            continue
    url = url.lower()
    if url[-4:] == ".gif" or url[-4:] == ".jpg" or url[-5:] == ".jpeg" or url[-4:] == ".png" or url[-4:] == ".css" or url[-3:] == ".js" or url[-4:] == ".mp3" or url[-4:] == ".mp4" or url[-4:] == ".ogg" or url[-5:] == ".midi" or url[-4:] == ".swf":
        continue
    # print("url", num, url, pageSize)

    # Ignore robots.txt when processing records
    if url[-11:] == "/robots.txt":
        continue
    payload = record.content_stream().read()
    payloads = []

    # Extract payloads (XML) from non-HTML document formats
    if url[-4:] == ".pdf" or ((record.http_headers is not None and record.http_headers.get_header('Content-Type') is not None) and "application/pdf" in record.http_headers.get_header('Content-Type')):
        if options.pdfextract:
            payloads = pdfextract(payload, extractor)
            # payloads = pdfextract_shell(payload)
        else:
            payloads = pdf2html(payload)
    elif url[-4:] == ".odt" or url[-4:] == ".ods" or url[-4:] == ".odp":
        payloads = openoffice2html(payload)
    elif url[-5:] == ".docx" or url[-5:] == ".pptx" or url[-5:] == ".xlsx":
        payloads = office2html(payload)
    elif url[-5:] == ".epub":
        payloads = epub2html(payload)
    else:
        payloads = [payload]

    date = record.rec_headers.get_header('WARC-Date')
    recordId = record.rec_headers.get_header('WARC-Record-ID')
    for payload in payloads:
        # We convert into UTF8 first of all
        orig_encoding, text = convert_encoding(payload)
        logging.info("Processing document: " + url)
        if orig_encoding is None:
            logging.info("Encoding of document " + url + " could not be identified")

        if len(text) > 0:
            # HTML is then normalized
            logging.info(url + ": cleaning html")
            tree = ""
            try:
                cleanhtml = cleaner.clean_html(re.sub('encoding *= *"[^"]+"', '', text, flags=re.IGNORECASE))
                tree = ftfy.fix_text(cleanhtml, fix_entities=False, fix_character_width=False)
            except Exception as ex:
                sys.stderr.write(str(ex) + "\n")
                continue
            cleantree = tree.replace("&#160;", " ")
            cleantree = cleantree.replace("\t", " ")

            # lang id
            # printable_str = ''.join(x for x in cleantree if x in string.printable)
            logging.info(url + ": detecting language")
            lang = guess_lang_from_data2(tree)
            if (len(languages) > 0 and lang not in languages) or (lang in banned):
                logging.info("Language of document " + url + ": " + lang + ". Not among searched languages.")
            else:
                # If enabled, remove boilerplate HTML
                if options.boilerpipe:
                    logging.info(url + ": deboiling html")
                    extractor = ExtrB(extractor='ArticleExtractor', html=cleantree)
                    deboiled = extractor.getHTML()
                else:
                    deboiled = cleantree

                # We compute MD5 on the HTML (either normalized one or after boilerpipe if enabled): if we get duplicate
                # files we discard them
                c = hashlib.md5()
                c.update(deboiled.encode())
                # print("hash", c.hexdigest(), url)
                # checking for duplicate content (duplicates are discarded)
                if c.hexdigest() in seen_md5:
                    logging.info("Repeated file:\t" + url + "\tfirst occurrence\t" + seen_md5[c.hexdigest()])
                    pass
                else:
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
                            print("Tree structure issues in HTML/XML. Ignoring this document")
                            continue
                        for tag in tree.css('script'):
                            tag.decompose()
                        for tag in tree.css('style'):
                            tag.decompose()
                        for tag in tree.css('img'):
                            tag.decompose()
                        if tree.body is None:
                            print("Body is empty. Ignoring this document")
                            continue
                        plaintext = tree.body.text(separator='\n')
                    plaintext = re.sub(r"\n+", "\n",
                                       re.sub(r" *\n *", "\n", re.sub(r" +", " ", re.sub(r"\r", "", plaintext))))

                    if len(plaintext) > 0:
                        seen_md5[c.hexdigest()] = c.hexdigest()
                        # Guessing MIME of the file (checked on original content)
                        logging.info(url + ": Getting mime")
                        mime = magic.from_buffer(text, mime=True)

                        if not options.xzlang:
                            mimeFile.write(mime.encode() + b"\n")

                            urlFile.write(url.encode() + b"\n")
                            langFile.write(lang.encode() + b"\n")
                            encodingFile.write(orig_encoding.encode() + b"\n")

                            b64norm = base64.b64encode(cleantree.encode())
                            normHtmlFile.write(b64norm + b"\n")

                            if options.boilerpipe:
                                b64deboil = base64.b64encode(deboiled.encode())
                                deboilFile.write(b64deboil + b"\n")

                            b64text = base64.b64encode(html.unescape(plaintext).encode())
                            plainTextFile.write(b64text + b"\n")

                        # append to language specific file
                        if options.xzlang:
                            langfile = lzma.open(options.outDir + "/" + options.prefix + lang + ".xz", "a")
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
        num += 1
if not options.xzlang:
    urlFile.close()
    langFile.close()
    encodingFile.close()
    mimeFile.close()
    normHtmlFile.close()
    plainTextFile.close()
# Boilerpipe cleaning is optional
if options.boilerpipe:
    deboilFile.close()
