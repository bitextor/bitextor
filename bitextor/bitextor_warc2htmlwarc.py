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

from warcio.archiveiterator import ArchiveIterator
from warcio.warcwriter import WARCWriter
from warcio.statusandheaders import StatusAndHeaders
import sys
import argparse
import cchardet
import re
import logging
import lzma
import subprocess
import zipfile
import io
from io import BytesIO


def convert_encoding(data):
    encoding = cchardet.detect(data)['encoding']
    decoded = ''
    # if detected encoding fails (or is None), try some common encodings
    for enc in [encoding, 'utf-8', 'iso-8859-1', 'windows‑1252']:
        try:
            decoded = data.decode(encoding)
            return enc, decoded
        except BaseException:
            pass
    return None, ''


def pdf2html(data):
    pconverter = subprocess.Popen(["pdftohtml", "-i", "-stdout", "-", "-"],
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    converter_stdout, error = pconverter.communicate(input=data)
    return [converter_stdout.replace(b"&#160;", b" ")]


def pdfextract(data, pdfextractor):
    pdfextractor.setData(data)
    try:
        return [bytes(pdfextractor.getHTML(), 'utf8')]
    except BaseException:
        return [b""]


def openoffice2html(data):
    datastream = io.BytesIO(data)
    try:
        openoffice_file = zipfile.ZipFile(datastream)
        return [openoffice_file.read('content.xml')]
    except zipfile.BadZipFile:
        return []


def office2html(data):
    datastream = io.BytesIO(data)
    try:
        office_file = zipfile.ZipFile(datastream)
        # word/document.xml, ppt/slides/slide*.xml, xl/sharedStrings.xml
        xmls = []
        for xml in office_file.namelist():
            if "word/document.xml" == xml or "ppt/slides/slide" == xml[0:16] or "xl/sharedStrings.xml" == xml:
                try:
                    xmls.append(office_file.read(xml))
                except Exception as ex:
                    continue
        return xmls
    except zipfile.BadZipFile:
        return []


def epub2html(data):
    datastream = io.BytesIO(data)
    try:
        epub_file = zipfile.ZipFile(datastream)
        # EPUB/*html
        xmls = []
        for xml in epub_file.namelist():
            if "ml" == xml[-2:]:
                try:
                    xmls.append(epub_file.read(xml))
                except Exception as ex:
                    continue
        return xmls
    except zipfile.BadZipFile:
        return []


oparser = argparse.ArgumentParser(
    description="Script that takes every record in a WARC file and runs basic preprocessing, which includes: HTML"
                "normalization, deduplication. The result is a WARC file.")
oparser.add_argument('-v', "--verbose", action="store_true", default=False,
                     help="Produce additional information about preprocessing through stderr.")
oparser.add_argument('-o', '--output', dest='output', help='Output WARC file', default=sys.stdout)
oparser.add_argument('-i', '--input', dest='input', help='Input WARC file', default=sys.stdin)
oparser.add_argument('--only-broader', dest='onlybroader', action="store_true",
                     help="Only outputs broader document format records", default=False)
oparser.add_argument('--pdfextract', action="store_true", help='Use pdf-extract engine or pdftohtml for PDFs',
                     default=False)
oparser.add_argument('--pe_configfile', dest='configFile', default="",
                     help='PDFExtract configuration file for language model paths')

oparser.add_argument('--sentence_join_path', dest='sentenceJoinPath', help='sentence-join.py path', default="")
oparser.add_argument('--kenlm_path', dest='kenlmPath', help='kenlm binary folder path', default="")
oparser.add_argument('--pdfpass', dest='pdfpass', help='Pass PDFs verbatim to file', default=None)
oparser.add_argument('--ftfy', action='store_true', help='User fix-text-for-you to fix possible encoding problems',
                     default=False)
oparser.add_argument('--cleanhtml', action='store_true', help='Clean HTML to remove javascript, css and head tags',
                     default=False)
oparser.add_argument('--disable-output-gzip', dest='disable_output_gzip', action='store_true',
                     help='Disable compression of output WARC')
oparser.add_argument('--disable-pdfs-gzip', dest='disable_pdfs_gzip', action='store_true',
                     help='Disable compression of PDFs WARC (if --pdfpass is enabled)')
options = oparser.parse_args()

logging.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=logging.INFO if options.verbose else logging.ERROR,
    datefmt='%Y-%m-%d %H:%M:%S')

f = None
fo = None
po = None
extractor = None

if options.input == sys.stdin or options.input == '-':
    f = ArchiveIterator(sys.stdin.buffer)
elif options.input[-3:] == ".xz":
    f = ArchiveIterator(lzma.open(options.input, 'r'))
elif options.input[-3:] == ".gz":
    f = ArchiveIterator(open(options.input, 'rb'))
else:
    f = ArchiveIterator(open(options.input, 'rb'))

if options.output == sys.stdout or options.output == '-':
    fo = WARCWriter(sys.stdout.buffer, gzip=True)
else:
    fo = WARCWriter(open(options.output, 'wb'), gzip=not options.disable_output_gzip)

if options.pdfpass is not None:
    po = WARCWriter(open(options.pdfpass, 'wb'), gzip=not options.disable_pdfs_gzip)

if not options.pdfpass and options.pdfextract:
    import jpype
    from pdfextract.extract import Extractor as ExtrP
    extractor = ExtrP(
        configFile=options.configFile,
        sentenceJoinPath=options.sentenceJoinPath,
        kenlmPath=options.kenlmPath)

cleaner = None
if options.cleanhtml:
    from lxml.html.clean import Cleaner
    cleaner = Cleaner(style=True, links=True, add_nofollow=True, page_structure=False, safe_attrs_only=False)

if options.output == sys.stdout or options.output == '-':
    filename = ""
else:
    filename = options.output

fo.write_record(
    fo.create_warcinfo_record(
        filename=filename,
        info={
            'software': 'bitextor/bitextor-warc2htmlwarc.py',
            'format': 'WARC File Format 1.0'}))

for record in f:
    # Initial checks
    if record.rec_type != 'response' and record.rec_type != 'resource':
        continue
    if not record.rec_headers.get_header('WARC-Target-URI'):
        url = None
    elif record.rec_headers.get_header('WARC-Target-URI')[0] == '<' and record.rec_headers.get_header('WARC-Target-URI')[-1] == '>':
        url = record.rec_headers.get_header('WARC-Target-URI')[1:-1]
    else:
        url = record.rec_headers.get_header('WARC-Target-URI')
    if url == "unknown" or not url:
        logging.info("Skipping page with unknown URL")
        continue
    if record.rec_headers.get_header('Content-Type') is None:
        continue
    if "text/dns" in record.rec_headers.get_header('Content-Type'):
        continue
    pageSize = int(record.rec_headers.get_header('Content-Length'))
    if pageSize > 5242880:
        logging.info("Skipping page, over limit. " + str(pageSize) + " " + url)
        continue
    if record.http_headers is not None and record.http_headers.get_header('Content-Type') is not None:
        if "image/" in record.http_headers.get_header('Content-Type') \
                or "audio/" in record.http_headers.get_header('Content-Type') \
                or "video/" in record.http_headers.get_header('Content-Type') \
                or "text/x-component" in record.http_headers.get_header('Content-Type') \
                or "text/x-js" in record.http_headers.get_header('Content-Type') \
                or "text/javascript" in record.http_headers.get_header('Content-Type') \
                or "application/x-javascript" in record.http_headers.get_header('Content-Type') \
                or "text/css" in record.http_headers.get_header('Content-Type') \
                or "application/javascript" in record.http_headers.get_header('Content-Type') \
                or "application/x-shockwave-flash" in record.http_headers.get_header('Content-Type') \
                or "application/octet-stream" in record.http_headers.get_header('Content-Type') \
                or "application/x-font-ttf" in record.http_headers.get_header('Content-Type'):
            continue

    url = url.replace('\t', ' ')
    if url[-4:] == ".gif" or url[-4:] == ".jpg" or url[-5:] == ".jpeg" or url[-4:] == ".png" \
            or url[-4:] == ".css" or url[-3:] == ".js" or url[-4:] == ".mp3" or url[-4:] == ".mp4" \
            or url[-4:] == ".ogg" or url[-5:] == ".midi" or url[-4:] == ".swf":
        continue

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
        # Transfer-Encoding: chunked header causes error with giawarc
        http_headers.remove_header("Transfer-Encoding")
        try:
            http_headers.to_ascii_bytes()
        except UnicodeEncodeError:
            # if header is non ascii, create a new header, with status code only
            # content length and content type will be filled before writing
            http_headers = StatusAndHeaders(record.http_headers.get_statuscode(), [])

    bdf = True
    # Extract payloads (XML) from non-HTML document formats
    if url[-4:] == ".pdf" \
            or ((record.http_headers is not None and record.http_headers.get_header('Content-Type') is not None)
                and "application/pdf" in record.http_headers.get_header('Content-Type')
                ):
        if options.pdfpass:
            new_record = po.create_warc_record(
                uri=url,
                record_type=record_type,
                warc_content_type=record.content_type,
                payload=BytesIO(payload),
                http_headers=http_headers)
            po.write_record(new_record)
            continue  # do not process further!
        if options.pdfextract:
            payloads = pdfextract(payload, extractor)
        else:
            payloads = pdf2html(payload)
    elif url[-4:] == ".odt" or url[-4:] == ".ods" or url[-4:] == ".odp":
        payloads = openoffice2html(payload)
    elif url[-5:] == ".docx" or url[-5:] == ".pptx" or url[-5:] == ".xlsx":
        payloads = office2html(payload)
    elif url[-5:] == ".epub":
        payloads = epub2html(payload)
    else:
        bdf = False
        if options.onlybroader:
            payloads = []
        else:
            payloads = [payload]

    date = record.rec_headers.get_header('WARC-Date')
    recordId = record.rec_headers.get_header('WARC-Record-ID')
    for payload in payloads:
        if not payload:
            continue

        logging.info("Processing document: " + url)
        # We convert into UTF8 first of all
        orig_encoding, text = convert_encoding(payload)

        if orig_encoding is None:
            logging.info("Encoding of document " + url + " could not be identified")
            continue

        text = re.sub('encoding *= *"[^"]+"', '', text, flags=re.IGNORECASE)
        if len(text.strip()) == 0:
            continue

        clean_html = ""
        tree = ""
        try:
            if options.cleanhtml:
                # HTML is then normalized
                logging.info(url + ": cleaning HTML")
                clean_html = cleaner.clean_html(text)
            else:
                clean_html = text

            if options.ftfy:
                import ftfy
                tree = ftfy.fix_text(clean_html, unescape_html=False, fix_character_width=False)
            else:
                tree = clean_html

        except Exception as ex:
            logging.info("Skipping " + url + ": " + str(ex))
            continue
        clean_tree = tree.replace("&#160;", " ")
        clean_tree = clean_tree.replace("\t", " ")
        clean_tree = clean_tree.encode('utf-8')
        if http_headers:
            http_headers.replace_header('Content-Length', str(len(clean_tree)))
            http_headers.replace_header('Content-Type', 'text/html')
        elif not http_headers and bdf:
            # for broader document formats without HTTP header create a fake one
            # to make it easier to distinguish between binary and processed documents downstream (warc2text)
            record_type = 'response'
            http_headers = StatusAndHeaders(
                statusline="200 OK",
                protocol="HTTP/1.1",
                headers=[('Content-Type', 'text/html'), ('Content-Length', str(len(clean_tree)))]
            )

        new_record = fo.create_warc_record(
            uri=url,
            record_type=record_type,
            warc_content_type=record.content_type,
            payload=BytesIO(clean_tree),
            http_headers=http_headers)
        fo.write_record(new_record)
