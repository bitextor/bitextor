#!/usr/bin/env python3

import warc
import base64
import sys

f = warc.WARCFile(fileobj=sys.stdin.buffer)
for record in f:
    print(base64.b64encode(record.payload.read()).decode('utf8')+"\t"+record.url+"\t"+record.date)

