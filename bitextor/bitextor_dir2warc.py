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

import os.path
import re
import sys
import argparse
import io
from io import BytesIO
from warcio.warcwriter import WARCWriter
from dateutil.parser import parse
from datetime import datetime

oparser = argparse.ArgumentParser(description="Script that takes a list of file paths from HTTrack crawled folder")
options = oparser.parse_args()

reader = sys.stdin
writer = WARCWriter(sys.stdout.buffer, gzip=True)

for fline in reader:
    filepath = fline.strip()
    if os.path.isfile(filepath):  # protect again extraneous 'Binary file (standard input) matches' at the end of stream  
        url = filepath.split("/")[-1]
        date = None
        with open(filepath, 'rb') as content_file:
            content = content_file.read()

        if date is None:
            dvalue = datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ')
        else:
            try:
                dvalue = parse(date.decode("utf8")).strftime('%Y-%m-%dT%H:%M:%SZ')
            except ValueError:
                dvalue = datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ')
   
        with open(filepath, 'rb') as content_file:
            record = writer.create_warc_record(uri=url, record_type="response", warc_content_type="application/http; msgtype=response", payload=content_file, http_headers="")

        writer.write_record(record)
