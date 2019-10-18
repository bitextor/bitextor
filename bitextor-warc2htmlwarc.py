#!/usr/bin/env python3

from warcio.archiveiterator import ArchiveIterator
from warcio.warcwriter import WARCWriter
from warcio.statusandheaders import StatusAndHeaders
import sys
import argparse
import cchardet
import re
import ftfy
from lxml.html.clean import Cleaner
import jpype
import os
import imp
import logging
import lzma
import subprocess
import zipfile
import io
from io import BytesIO


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
from pdfextract.extract import Extractor as ExtrP


def convert_encoding(data):
    encoding = cchardet.detect(data)['encoding']
    if encoding is None:
        encoding = "utf-8"
    if len(data) > 0:
        # We convert, even if the text is detected to be UTF8 so, if it is an error and conversion fails, the error
        # is caught here
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
    try:
        return [bytes(extractor.getHTML(), 'utf8')]
    except:
        return [b""]


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
    description="Script that takes every record in a WARC file and runs basic preprocessing, which includes: HTML"
                "normalization, deduplication. The result is a WARC file.")
oparser.add_argument("--verbose", action="store_true", default=False,
                     help="Produce additional information about preprocessing through stderr.")
oparser.add_argument('--output', dest='output', help='Output WARC file', default=sys.stdout)
oparser.add_argument('--input', dest='input', help='Input WARC file', default=sys.stdin)
oparser.add_argument('--pdfextract', action="store_true", help='Use pdf-extract engine or pdftohtml for PDFs',
                     default=False)
options = oparser.parse_args()

logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO if options.verbose else logging.ERROR, datefmt='%Y-%m-%d %H:%M:%S')
f = None
fo = None
if options.input[-3:] == ".xz":
    f = ArchiveIterator(lzma.open(options.input, 'r'))
elif options.input[-3:] == ".gz":
    f = ArchiveIterator(open(options.input, 'rb'))
elif options.input == sys.stdin:
    f = ArchiveIterator(options.input.buffer)
else:
    f = ArchiveIterator(open(options.input, 'rb'))

if options.output == sys.stdout:
    fo = WARCWriter(options.output.buffer, gzip=True)
else:
    fo = WARCWriter(open(options.output,'wb'), gzip=True)

if options.pdfextract:
    extractor = ExtrP()

cleaner = Cleaner(style=True, links=True, add_nofollow=True, page_structure=False, safe_attrs_only=False)

if options.output == sys.stdout:
    filename = options.input
else:
    filename = options.output

fo.write_record(fo.create_warcinfo_record(filename=filename, info={'software': 'bitextor/bitextor-warc2htmlwarc.py', 'format': 'WARC File Format 1.0'}))

for record in f:
    # Initial checks
    if record.rec_type != 'response' and record.rec_type != 'resource':
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
    url = url.replace('\t',' ')
    if url[-4:] == ".gif" or url[-4:] == ".jpg" or url[-5:] == ".jpeg" or url[-4:] == ".png" or url[-4:] == ".css" or url[-3:] == ".js" or url[-4:] == ".mp3" or url[-4:] == ".mp4" or url[-4:] == ".ogg" or url[-5:] == ".midi" or url[-4:] == ".swf":
        continue
    # print("url", num, url, pageSize)

    # Ignore robots.txt when processing records
    if url[-11:] == "/robots.txt":
        continue
    payload = record.content_stream().read()
    payloads = []

    if not record.http_headers or record.http_headers.to_str()[:7] != "HTTP/1.":
        if record.http_headers:
            payload = record.http_headers.to_bytes() + payload
        record_type = 'resource'
        http_headers = None
    else:
        record_type = 'response'
        http_headers = record.http_headers
        try:
            http_headers.to_ascii_bytes()
        except UnicodeEncodeError:
            # if header is non ascii, create a new header, with status code only
            # content length and content type will be filled before writing
            http_headers = StatusAndHeaders(record.http_headers.get_statuscode(), [])

    # Transfer-Encoding: chunked header causes error with giawarc
    http_headers.remove_header("Transfer-Encoding")

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
            continue

        text = re.sub('encoding *= *"[^"]+"', '', text, flags=re.IGNORECASE)
        if len(text.strip()) == 0:
            continue
        # HTML is then normalized
        logging.info(url + ": cleaning html")
        tree = ""
        try:
            clean_html = cleaner.clean_html(text)
            tree = ftfy.fix_text(clean_html, fix_entities=False, fix_character_width=False)
        except Exception as ex:
            sys.stderr.write(str(ex) + "\n")
            continue
        clean_tree = tree.replace("&#160;", " ")
        clean_tree = clean_tree.replace("\t", " ")
        clean_tree = re.sub(r"\n+", "\n", re.sub(r" *\n *", "\n", re.sub(r" +", " ", re.sub(r"\r", "", clean_tree))))
        clean_tree = clean_tree.encode('utf-8')
        if http_headers:
            http_headers.replace_header('Content-Length', str(len(clean_tree)))
            http_headers.replace_header('Content-Type', 'text/html')
        new_record = fo.create_warc_record(uri=url, record_type=record_type, warc_content_type=record.content_type, payload=BytesIO(clean_tree), http_headers=http_headers)
        fo.write_record(new_record)

