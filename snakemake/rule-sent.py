#!/usr/bin/env python3

import os
import sys
import argparse
import tldextract
import lzma
import shutil

print("# Starting")

oparser = argparse.ArgumentParser(description="Create script to delete corrupted files")
oparser.add_argument('--input-dir', dest='inDir', help='Transient directory', required=True)
#oparser.add_argument('--lang1', dest='lang1', help='language', required=True)
#oparser.add_argument('--lang2', dest='lang2', help='language', required=True)
options = oparser.parse_args()

output = "sent.xz"
with open(output,'wb') as wfd:

  for dir in os.listdir(options.inDir):
    #print("dir", dir)
    dir = "{inDir}/{dir}".format(inDir=options.inDir, dir=dir)

    # LANG.XZ == 0
    file = "{dir}/bleualign.elrc.xz".format(dir=dir)
    if os.path.isfile(file):
      print(" ll {file}".format(file=file))

      with open(file,'rb') as fd:
        shutil.copyfileobj(fd, wfd, 1024*1024*10)
                            
print("# Finished")
