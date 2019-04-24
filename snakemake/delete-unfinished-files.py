#!/usr/bin/env python3

import os
import sys
import argparse
import tldextract



print("Starting")

oparser = argparse.ArgumentParser(description="Hello")
oparser.add_argument('--input-dir', dest='inDir', help='Root directory of domains', required=True)
options = oparser.parse_args()

for dir in os.listdir(options.inDir):
  #print("dir", dir)
  dir = "{inDir}/{dir}".format(inDir=options.inDir, dir=dir)
  
  file = "{dir}/lang.xz".format(dir=dir)
  if os.path.isfile(file):
      size = os.path.getsize(file)
      if size == 0:
          print("# {dir}".format(dir=dir))
          print(" rm -rf {dir}/encoding.xz {dir}/lang.xz {dir}/mime.xz {dir}/normalized_html.xz {dir}/plain_text.xz {dir}/url.xz {dir}/docalign".format(dir=dir))
          
print("Finished")
