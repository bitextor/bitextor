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


parser = argparse.ArgumentParser(description="Script that takes a list of directories and creates a WARC with content of each directory",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

def get_files(dir_path):
    for element in os.listdir(dir_path):
        if os.path.isfile(os.path.join(dir_path, element)):
            files.append(os.path.join(dir_path, element))
        elif os.path.isdir(os.path.join(dir_path, element)) and not os.path.islink(os.path.join(dir_path, element)):
            get_files(os.path.join(dir_path, element))

reader = sys.stdin
writer = WARCWriter(sys.stdout.buffer, gzip=True)

for dline in reader:
    files = []
    get_files(dline.strip())
    
    for filepath in files:
        url =  '/'.join(filepath.split("/")[-2:])
        date = datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ')

        with open(filepath, 'rb') as content_file:
            record = writer.create_warc_record(uri=url, record_type="response", warc_content_type="application/http; msgtype=response", payload=content_file, http_headers="")

        writer.write_record(record)
