#!/usr/bin/env python3

import os.path
import re
import sys
import argparse
import warc

oparser = argparse.ArgumentParser(description="Script that takes a list of file paths from HTTrack crawled folder")
options = oparser.parse_args()

reader = sys.stdin

for fline in reader:
    filepath=fline.strip()
    if os.path.isfile(filepath): # protect again extraneous 'Binary file (standard input) matches' at the end of stream
        content=None
        url=None
        with open(filepath, 'rb') as content_file:
            content = content_file.read()
        for line in content.split(b"\n"):
          if re.search(rb'<!-- Mirrored from ', line):
              url = re.sub(rb'.*<!-- Mirrored from ', b'', re.sub(rb' by HTTrack Website Copier.*', b'', line))
              break
        if url == None:
            warc_record = warc.WARCRecord(payload=content,headers={"WARC-Target-URI":"unknown"})
        else:
            warc_record = warc.WARCRecord(payload=content,headers={"WARC-Target-URI":url.decode("utf8")})
        f = warc.WARCFile(fileobj=sys.stdout.buffer)
        f.write_record(warc_record)

