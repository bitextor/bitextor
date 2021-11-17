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

import html
from warcio.archiveiterator import ArchiveIterator
import base64
import argparse
import cchardet
import magic
import re
from bs4 import BeautifulSoup
import os
import importlib
import logging
import lzma
import gzip
from selectolax.parser import HTMLParser
from html.parser import HTMLParser as HTMLTokenizer
import mmh3
import sys
import html5lib
import lxml
from lxml import etree
from lxml import html as _lxml_html


def remove_control_characters(html):
    # type: (t.Text) -> t.Text
    """
    Strip invalid XML characters that `lxml` cannot parse.
    """
    # See: https://github.com/html5lib/html5lib-python/issues/96
    #
    # The XML 1.0 spec defines the valid character range as:
    # Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
    #
    # We can instead match the invalid characters by inverting that range into:
    # InvalidChar ::= #xb | #xc | #xFFFE | #xFFFF | [#x0-#x8] | [#xe-#x1F] | [#xD800-#xDFFF]
    #
    # Sources:
    # https://www.w3.org/TR/REC-xml/#charsets,
    # https://lsimons.wordpress.com/2011/03/17/stripping-illegal-characters-out-of-xml-in-python/
    def strip_illegal_xml_characters(s, default, base=10):
        # Compare the "invalid XML character range" numerically
        n = int(s, base)
        if n in (0xb, 0xc, 0xFFFE, 0xFFFF) or 0x0 <= n <= 0x8 or 0xe <= n <= 0x1F or 0xD800 <= n <= 0xDFFF:
            return ""
        return default

    # We encode all non-ascii characters to XML char-refs, so for example "💖" becomes: "&#x1F496;"
    # Otherwise we'd remove emojis by mistake on narrow-unicode builds of Python
    html = html.decode('utf8').encode("ascii", "xmlcharrefreplace").decode("utf-8")
    html = re.sub(r"&#(\d+);?", lambda c: strip_illegal_xml_characters(c.group(1), c.group(0)), html)
    html = re.sub(
        r"&#[xX]([0-9a-fA-F]+);?",
        lambda c: strip_illegal_xml_characters(
            c.group(1),
            c.group(0),
            base=16
        ),
        html)
    html = ILLEGAL_XML_CHARS_RE.sub("", html)
    return html


# A regex matching the "invalid XML character range"
ILLEGAL_XML_CHARS_RE = re.compile(r"[\x00-\x08\x0b\x0c\x0e-\x1F\uD800-\uDFFF\uFFFE\uFFFF]")


class SimpleParser(HTMLTokenizer):
    startendNL = [
        "ul",
        "ol",
        "dl",
        "tr",
        "p",
        "div",
        "li",
        "dd",
        "dt",
        "th",
        "td",
        "h1",
        "h2",
        "h3",
        "h4",
        "h5",
        "h6",
        "article",
        "aside",
        "blockquote",
        "details",
        "summary",
        "figcaption",
        "footer",
        "form",
        "header",
        "legend",
        "main",
        "nav",
        "pre",
        "section"]
    selfNL = ["br", "hr"]
    noText = ["script", "noscript", "style"]
    lastTok = ""
    parsed = ""

    def handle_starttag(self, tag, attrs):
        if tag in self.startendNL or tag in self.selfNL:
            self.parsed = self.parsed + "\n"
        self.lastTok = tag

    def handle_endtag(self, tag):
        if tag in self.startendNL:
            self.parsed = self.parsed + "\n"
        else:
            self.parsed = self.parsed + " "

    def handle_startendtag(self, tag, attrs):
        if tag in self.selfNL:
            self.parsed = self.parsed + "\n"

    def handle_data(self, data):
        if self.lastTok not in self.noText:
            newdata = data.replace("\r\n", " ").replace("\n", " ")
            self.parsed = self.parsed + newdata

    def get_text(self):
        return self.parsed.strip() + "\n"


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
        # We convert, even if the text is detected to be UTF8 so,
        # if it is an error and conversion fails, the error is caught here
        for enc in [encoding, 'utf-8', 'iso-8859-1', 'windows‑1252']:
            try:
                return enc, data.decode(enc)
            except BaseException:
                pass
    return None, ''


def open_xz_or_gzip(path, mode):
    if path[-3:] == '.gz':
        return gzip.open(path, mode)
    elif path[-3:] == '.xz':
        return lzma.open(path, mode)
    else:
        return open(path, mode)


def open_output_files(options, lang, files_dict):
    if not options.xzlang and lang not in files_dict:
        if not os.path.exists(f"{options.outDir}/{lang}"):
            os.makedirs(f"{options.outDir}/{lang}")
        urlFile = open_xz_or_gzip(f"{options.outDir}/{lang}/url.{options.compression}", "w")
        encodingFile = open_xz_or_gzip(f"{options.outDir}/{lang}/encoding.{options.compression}", "w")
        mimeFile = open_xz_or_gzip(f"{options.outDir}/{lang}/mime.{options.compression}", "w")
        normHtmlFile = open_xz_or_gzip(f"{options.outDir}/{lang}/normalized_html.{options.compression}", "w")
        plainTextFile = open_xz_or_gzip(f"{options.outDir}/{lang}/plain_text.{options.compression}", "w")
        if options.boilerpipe:
            deboilFile = open_xz_or_gzip(f"{options.outDir}/{lang}/deboilerplate_html.{options.compression}", "w")
            files_dict[lang] = {
                "urlFile": urlFile,
                "encodingFile": encodingFile,
                "mimeFile": mimeFile,
                "normHtmlFile": normHtmlFile,
                "plainTextFile": plainTextFile,
                "deboilFile": deboilFile
            }
        else:
            if not os.path.exists(f"{options.outDir}/{lang}/deboilerplate_html.{options.compression}") \
                    and not os.path.islink(f"{options.outDir}/{lang}/deboilerplate_html.{options.compression}"):
                os.symlink(
                    f"normalized_html.{options.compression}",
                    f"{options.outDir}/{lang}/deboilerplate_html.{options.compression}"
                )
            files_dict[lang] = {
                "urlFile": urlFile,
                "encodingFile": encodingFile,
                "mimeFile": mimeFile,
                "normHtmlFile": normHtmlFile,
                "plainTextFile": plainTextFile}


oparser = argparse.ArgumentParser(
    description="Script that takes every record in a WARC file and runs preprocessing, which includes: HTML"
                "normalization, deduplication, MIME and language identification, and boilerplate removing. The result"
                "of each pre-processing step is stored in a XZ compressed file in the output directory.")
oparser.add_argument("--verbose", action="store_true", default=False,
                     help="Produce additional information about preprocessing through stderr.")
oparser.add_argument("--boilerpipe", action="store_true", default=False,
                     help="Use boilerpipe bodytext to do the de-boiling")
oparser.add_argument("--parser", dest="parser", default="bs4", choices={'bs4', 'modest', 'lxml', 'simple'},
                     help="Use 'HTML tokenizer', 'modest', 'bs4' or 'lxml' (using html5lib tree) parser to extract relevant text from HTML. By default 'bs4' is used")
oparser.add_argument("--html5lib", action="store_true", default=False, help="Process HTML tree with html5lib")
oparser.add_argument('--output-dir', dest='outDir', help='Output directory', required=True)
oparser.add_argument('--output_hash', dest='outputHash', help='Output path for Murmur Hash of plain texts')
oparser.add_argument('--input_hash', dest='inputHash',
                     help='Input path for previous Bitextor Murmur Hash plain texts file')
oparser.add_argument('--lang1', dest='l1', help='Language l1 in the crawl', default=None)
oparser.add_argument('--lang2', dest='l2', help='Language l2 in the crawl', default=None)
oparser.add_argument('--input', dest='input', help='Input WARC file', default=sys.stdin)
oparser.add_argument('--xzlang', action="store_true", help='Separate output into different files by language',
                     default=False)
oparser.add_argument('--langs', dest="langs", default="",
                     help='List of languages to include or ignore (%%): l1,l2,%%l3,%%l4')
oparser.add_argument('--langid', dest="langid", default="cld2", help="Model used for language detection: cld2 or cld3")
oparser.add_argument('--compression', dest='compression', default='gz', choices={'xz', 'gz'},
                     help='Compression type for the output files')
options = oparser.parse_args()

logging.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=logging.INFO if options.verbose else logging.ERROR,
    datefmt='%Y-%m-%d %H:%M:%S'
)

if options.input == sys.stdin or options.input == '-':
    f = ArchiveIterator(sys.stdin.buffer)
elif options.input[-3:] == ".xz":
    f = ArchiveIterator(lzma.open(options.input, 'r'))
elif options.input[-3:] == ".gz":
    f = ArchiveIterator(open(options.input, 'rb'))
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

previous_crawl_hashes = set()

if not os.path.exists(options.outDir):
    os.makedirs(options.outDir)

if options.inputHash:
    with open_xz_or_gzip(options.inputHash, 'r') as fh:
        for line in fh:
            previous_crawl_hashes.add(int(line.strip()))

plainTextHashFile = None
if options.outputHash:
    plainTextHashFile = open_xz_or_gzip(options.outputHash, "w")

files_dict = dict()

ExtrB = None
if options.boilerpipe:
    import jpype
    if not jpype.isJVMStarted():
        jars = []
        for top, dirs, files in os.walk(
            os.path.dirname(importlib.machinery.PathFinder().find_module("boilerpipe").get_filename()) + '/data'
        ):
            for nm in files:
                if nm[-4:] == ".jar":
                    jars.append(os.path.join(top, nm))
        jpype.addClassPath(os.pathsep.join(jars))
        jpype.startJVM(jpype.getDefaultJVMPath(), convertStrings=False)
    from boilerpipe.extract import Extractor as ExtrB

for record in f:
    plaintext = ""
    # Initial checks
    if record.rec_type != 'response' and record.rec_type != 'resource':
        continue
    if record.rec_headers.get_header('WARC-Target-URI')[0] == '<' \
            and record.rec_headers.get_header('WARC-Target-URI')[-1] == '>':
        url = record.rec_headers.get_header('WARC-Target-URI')[1:-1]
    else:
        url = record.rec_headers.get_header('WARC-Target-URI')
    if url == "unknown":
        logging.info("Skipping page with unknown URL")
        continue
    url = url.replace('\t', ' ')
    if url[-4:] == ".gif" or url[-4:] == ".jpg" or url[-5:] == ".jpeg" or url[-4:] == ".png" or url[-4:] == ".css" \
            or url[-3:] == ".js" or url[-4:] == ".mp3" or url[-4:] == ".mp4" or url[-4:] == ".ogg" \
            or url[-5:] == ".midi" or url[-4:] == ".swf":
        continue

    # Ignore robots.txt when processing records
    if url[-11:] == "/robots.txt":
        continue

    payload = record.content_stream().read()

    # We convert into UTF8 first of all
    orig_encoding, text = convert_encoding(payload)

    # Fix HTML issues with html5lib if activated through parameters
    if options.html5lib or options.parser == "lxml":
        document = html5lib.parse(remove_control_characters(bytes(text, 'utf8')),
                                  treebuilder="lxml", namespaceHTMLElements=False)
        text = etree.tostring(document, encoding="utf8").decode('utf8')

    logging.info("Processing document: " + url)
    if orig_encoding is None:
        logging.info("Encoding of document " + url + " could not be identified")
        continue

    date = record.rec_headers.get_header('WARC-Date')
    recordId = record.rec_headers.get_header('WARC-Record-ID')

    if len(text.strip()) == 0:
        continue

    # lang id
    logging.info(url + ": detecting language")
    lang = ""

    if options.langid == "cld2":
        lang = guess_lang_from_data2(text)
        if (len(languages) > 0 and lang not in languages) or (lang in banned):
            logging.info("Language of document " + url + ": " + lang + ". Not among searched languages.")
            continue
        if lang == "un":
            logging.info("Language of document " + url + " could not be identified")
            continue
        open_output_files(options, lang, files_dict)

    # If enabled, remove boilerplate HTML
    if options.boilerpipe:
        logging.info(url + ": deboiling html")
        extractor = ExtrB(extractor='ArticleExtractor', html=text)
        deboiled = str(extractor.getHTML())
    else:
        deboiled = text

    # We compute a hash on the HTML (either normalized one or after boilerpipe if enabled):
    # if we get duplicate files we discard them
    html_hash = mmh3.hash(deboiled, signed=False)
    # checking for duplicate content (duplicates are discarded)
    if html_hash in seen_html:
        logging.info("Repeated file:\t" + url)
        continue

    # get text with beautifulsoup
    if options.parser == "bs4":
        logging.info(url + ": Getting text with BeautifulSoup")
        try:
            soup = BeautifulSoup(deboiled, "lxml")
        except Exception as ex:
            logging.info("Exception ocurred when processing " + url + " with BeautifulSoup")
            continue

        for script in soup(["script", "style", "img"]):
            script.extract()  # rip it out
        plaintext = soup.get_text()

    # or get text with 'modest' library
    elif options.parser == "modest":
        logging.info(url + ": Getting text with modest (selectolax)")
        try:
            tree = HTMLParser(deboiled)
        except BaseException:
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
        # TODO should separator='\n' be removed? It splits inline elements inside block elements...
        plaintext = tree.body.text(separator='\n')

    # or get text by moving through the lxml tree
    elif options.parser == "lxml":
        logging.info(url + ": Getting text with lxml")

        plaintext = lxml.html.document_fromstring(deboiled).text_content()
    # or use an HTML tokenizer
    else:
        logging.info(url + ": Getting text with HTML tokenizer")
        parser = SimpleParser()
        try:
            parser.feed(text)
            plaintext = parser.get_text()
        except BaseException:
            logging.info("Tree structure issues in HTML/XML. Ignoring this document")
            continue
    plaintext = re.sub(r"\n+", "\n",
                       re.sub(r" *\n *", "\n",
                              re.sub(r"[ \t\v\f]+", " ",
                                     re.sub(r"\r", "",
                                            plaintext.replace(u'\xa0', u' '))))).strip()
    if options.langid == "cld3":
        if plaintext:
            lang = guess_lang_from_data3(cld3model, plaintext)
            if (len(languages) > 0 and lang not in languages) or (lang in banned):
                logging.info("Language of document " + url + ": " + lang + ". Not among searched languages.")
                continue
            if lang == "un":
                logging.info("Language of document " + url + " could not be identified")
                continue
            open_output_files(options, lang, files_dict)
        else:
            continue

    plaintext_hash = mmh3.hash(plaintext, signed=False)

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
            header += "X-WARC-Record-ID: " + recordId + "\n"
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
