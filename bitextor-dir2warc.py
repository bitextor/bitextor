#!/usr/bin/env python3

import os.path
import re
import sys
import argparse
from warcio.warcwriter import WARCWriter
from warcio.statusandheaders import StatusAndHeaders
from dateutil.parser import parse
from datetime import datetime

oparser = argparse.ArgumentParser(description="Script that takes a list of file paths from HTTrack crawled folder")
options = oparser.parse_args()

reader = sys.stdin
writer = WARCWriter(sys.stdout.buffer, gzip=True)
http_headers = StatusAndHeaders('200 OK', [], protocol='HTTP/1.0')

for fline in reader:
    filepath = fline.strip()
    # sys.stderr.write("filepath=" + filepath + "\n")
    if os.path.isfile(filepath):  # protect again extraneous 'Binary file (standard input) matches' at the end of stream
        url = None
        date = None
        with open(filepath, 'rb') as content_file:
            content = content_file.read()
            for line in content.split(b"\n"):
                if re.search(rb'<!-- Mirrored from .* by HTTrack Website Copier.*\[.*\],', line):
                    url = re.sub(rb'.*<!-- Mirrored from ', b'', re.sub(rb' by HTTrack Website Copier.*', b'', line))
                    date = re.sub(rb'.+by HTTrack Website.+\[.+\][^,]*, ', b'', re.sub(rb' -->.*', b'', line))
                    break
        if date is None:
            dvalue = datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ')
        else:
            try:
                dvalue = parse(date.decode("utf8")).strftime('%Y-%m-%dT%H:%M:%SZ')
            except ValueError:
                dvalue = datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ')
        if url is None:
            urlStr = "unknown"
        else:
            try:
                urlStr = url.decode("utf8")
                # sys.stderr.write("HH1 " + urlStr + "\n")
            except:
                urlStr = "unknown-encoding"
                # sys.stderr.write("HH2 " + urlStr + "\n")
        with open(filepath, 'rb') as content_file:
            record = writer.create_warc_record(urlStr, 'response',
                                               payload=content_file,
                                               http_headers=http_headers)

        writer.write_record(record)
