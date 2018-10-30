#!__ENV__ python3

import re
import sys
import argparse
import warc

oparser = argparse.ArgumentParser(description="Script that takes a list of file paths from HTTrack crawled folder")
options = oparser.parse_args()

reader = sys.stdin

for line in reader:
    filepath=line.strip()
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

    warc_record = warc.WARCRecord(payload=content,headers={"WARC-Target-URI":url.decode("utf8")})
    f = warc.WARCFile(fileobj=sys.stdout.buffer)
    f.write_record(warc_record)

