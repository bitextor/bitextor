#!/usr/bin/env python3

import os.path
import re
import sys
import argparse
import warc
from dateutil.parser import parse
import dateutil
from datetime import datetime

oparser = argparse.ArgumentParser(description="Script that takes a list of file paths from HTTrack crawled folder")
options = oparser.parse_args()

reader = sys.stdin

for fline in reader:
    filepath = fline.strip()
    #sys.stderr.write("filepath=" + filepath + "\n")
    if os.path.isfile(filepath): # protect again extraneous 'Binary file (standard input) matches' at the end of stream
        content=None
        url=None
        date=None
        with open(filepath, 'rb') as content_file:
            content = content_file.read()
        for line in content.split(b"\n"):
          if re.search(rb'<!-- Mirrored from .* by HTTrack Website Copier.*\[.*\],', line):
              url = re.sub(rb'.*<!-- Mirrored from ', b'', re.sub(rb' by HTTrack Website Copier.*', b'', line))
              date = re.sub(rb'.+by HTTrack Website.+\[.+\][^,]*, ', b'', re.sub(rb' -->.*', b'', line))
              break
        if date == None:
            dvalue=datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ')
        else:
            dvalue=parse(date.decode("utf8")).strftime('%Y-%m-%dT%H:%M:%SZ')
        if url == None:
            warc_record = warc.WARCRecord(payload=content,headers={"WARC-Target-URI":"unknown","WARC-Date":dvalue})
        else:
            try:
                urlStr = url.decode("utf8")
                #sys.stderr.write("HH1 " + urlStr + "\n")
            except:
                urlStr = "unknown-encoding"
                #sys.stderr.write("HH2 " + urlStr + "\n")

            warc_record = warc.WARCRecord(payload=content,headers={"WARC-Target-URI":urlStr,"WARC-Date":dvalue})

        f = warc.WARCFile(fileobj=sys.stdout.buffer)
        f.write_record(warc_record)

