#!/usr/bin/env python3

import os
import sys
import argparse
import tldextract
import lzma

print("# Starting")

oparser = argparse.ArgumentParser(description="Hello")
oparser.add_argument('--input-dir', dest='inDir', help='Transient directory', required=True)
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

  #
  file = "{dir}/docalign/en.extracted.xz".format(dir=dir)
  if os.path.isfile(file):
    with lzma.open(file, 'rb') as f:
      try:
        for line in f:
          pass
          #print(line)
      except:
        print(" rm -rf {dir}/docalign/".format(dir=dir))
          
print("# Finished")
