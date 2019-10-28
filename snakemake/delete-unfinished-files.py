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

import os
import sys
import argparse
import tldextract
import lzma

print("# Starting")

oparser = argparse.ArgumentParser(description="Create script to delete corrupted files")
oparser.add_argument('--input-dir', dest='inDir', help='Transient directory', required=True)
oparser.add_argument('--lang', dest='lang', help='Not english language', required=True)
options = oparser.parse_args()

for dir in os.listdir(options.inDir):
  #print("dir", dir)
  dir = "{inDir}/{dir}".format(inDir=options.inDir, dir=dir)

  # LANG.XZ == 0
  file = "{dir}/lang.xz".format(dir=dir)
  if os.path.isfile(file):
      size = os.path.getsize(file)
      if size == 0:
          print("# {dir}".format(dir=dir))
          print(" rm -rf {dir}/encoding.xz {dir}/lang.xz {dir}/mime.xz {dir}/normalized_html.xz {dir}/plain_text.xz {dir}/url.xz {dir}/docalign".format(dir=dir))

  # extracted file is corrupted
  file = "{dir}/docalign/en.extracted.xz".format(dir=dir)
  if os.path.isfile(file):
    with lzma.open(file, 'rb') as f:
      try:
        for line in f:
          pass
          #print(line)
      except:
        print(" rm -rf {dir}/docalign/".format(dir=dir))
          
  # translated file is corrupted
  file = "{dir}/docalign/{lang}.customMT.extracted.deduped.translated.xz".format(dir=dir, lang=options.lang)
  if os.path.isfile(file):
    with lzma.open(file, 'rb') as f:
      try:
        for line in f:
          pass
          #print(line)
      except:
        print(" rm {file}".format(file=file))
  
print("# Finished")
