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
