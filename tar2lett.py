#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Reads downloaded website from tar file and writes lett format to be
processed by bitextor pipeline
"""

import sys
import tarfile
import base64
import re

from html2text import html2text
from textsanitizer import TextSanitizer

magic_number = "df6fa1abb58549287111ba8d776733e9"


def original_url(html):
    """ Extracts the original url from HTTrack comment """
    url = "unknown_url"
    for m in re.finditer(
            "<!-- Mirrored from ([^>]+) by HTTrack Website Copier", html):
        url = m.groups()[0]
        if not url.startswith('http://'):
            url = "http://" + url
    return url


def read_file2realurl(fh):
    file2realurl = {}
    if fh:
        for line in fh:
            filename, real_url = line.strip().split("\t")
            assert filename not in file2realurl
            file2realurl[filename] = real_url
    return file2realurl

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('tarfile', help='tarfile containing a webdir')
    parser.add_argument('srclang', help="source langauge e.g. en")
    parser.add_argument('tgtlang', help="target langauge e.g. fr")
    parser.add_argument('-file2realurl', type=argparse.FileType('r'),
                        help='given mapping between filenames and urls')
    parser.add_argument('-mapping', type=argparse.FileType('a'),
                        help='mapping between filenames and urls')
    parser.add_argument('-ignore_br', help="ignore <br> tags in HTML",
                        action='store_true', default=False)
    parser.add_argument('-filter-other-languages',
                        dest='filter',
                        help='remove pages in other languages',
                        action='store_true')

    mime_type = "text/html"
    enc = "charset=utf-8"

    args = parser.parse_args(sys.argv[1:])
    file2realurl = read_file2realurl(args.file2realurl)

    tar = tarfile.open(args.tarfile, "r:gz")

    for filenr, tarinfo in enumerate(tar):
        if not tarinfo.isreg():
            continue

        filename = tarinfo.name

        raw_data = tar.extractfile(tarinfo).read()
        data = TextSanitizer.to_unicode(raw_data, is_html=True, lang='auto')
        lang = TextSanitizer.guess_lang_from_data(
            data, is_html=True, default_lang=None)

        if not lang:
            sys.stderr.write("No langs for file %s\n" % filename)
            continue
        if args.filter and lang != args.srclang and lang != args.tgtlang:
            sys.stderr.write("Skipping %s because lang=%s\n" %
                             (filename, lang))
            continue
        

        text = html2text(data)
        data = data.encode('utf-8')  # utf-8 input expected

        original_uri = None
        if args.file2realurl is not None:
            if filename not in file2realurl:
                sys.stderr.write(
                    "Could not find %s in file2realurl\n" % filename)
            else:
                original_uri = file2realurl[filename]
        if original_uri is None:
            original_uri = original_url(data)
        if original_uri is "unknown_url":
            original_uri = "http://" + filename

        sys.stderr.write("Processed file Nr. %d : %s = %s\n" %
                         (filenr, filename, original_uri))

        sys.stdout.write("{l}\t{mime}\t{enc}\t{name}\t{html}\t{text}\n".format(
            l=lang,
            mime=mime_type,
            enc=enc,
            name=original_uri,
            html=base64.b64encode(data),
            text=base64.b64encode(text.encode('utf-8'))))

        if args.mapping:
            args.mapping.write(
                "%s\t%s\n" % (filename, original_uri))

    sys.stderr.write("Done. \n")
    tar.close()
