#!__ENV__ __PYTHON__

import re
import sys
import magic
import base64
import argparse
import warc

reload(sys)
sys.setdefaultencoding("UTF-8")

oparser = argparse.ArgumentParser(description="Script that takes a list of file paths from HTTrack crawled folder")
options = oparser.parse_args()

reader = sys.stdin

for line in reader:
    filepath=line.strip()
    content=None
    url=None
    with open(filepath, 'r') as content_file:
      content = content_file.read()
    for line in content.split("\n"):
      if re.search(r'<!-- Mirrored from ', line):
        url = re.sub(r'.*<!-- Mirrored from ', '', re.sub(r' by HTTrack Website Copier.*', '', line))
        break
    warc_record = warc.WARCRecord(payload=content,headers={"WARC-Target-URI":url})
    f = warc.WARCFile(fileobj=sys.stdout)
    f.write_record(warc_record)

