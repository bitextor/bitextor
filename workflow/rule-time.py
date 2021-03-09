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
from datetime import datetime
import statistics

print("# Starting")

oparser = argparse.ArgumentParser(description="Look in slurm log files to calculate run time for each rule")
oparser.add_argument('--input-dir', dest='inDir', help='Transient directory', required=True)
options = oparser.parse_args()

rules = {}

for fileName in os.listdir(options.inDir):
  if fileName[:6] == "slurm-":
    #print("fileName", fileName)
  
    f = open(fileName, 'r')
    lines = f.readlines()

    startDate = None
    timeTaken = None
    
    for line in lines:
      line = line.strip()

      if line[:5] == "rule ":
        ruleName = line[5:]
        #print("   ruleName", ruleName)
        
      elif line[0:1] == "[" and line[-1:] == "]":
        # date
        line = line[1:-1]
        #print("   ", line)
        datetimeObj = datetime.strptime(line, '%a %b %d %H:%M:%S %Y')
        if startDate == None:
          startDate = datetimeObj
        elif timeTaken == None:
          timeTaken = datetimeObj - startDate
          timeTaken = timeTaken.total_seconds()
          #print("   timeTaken", timeTaken)
        else:
          assert(False)

    if ruleName is not None and timeTaken is not None:
      if ruleName not in rules:
        rules[ruleName] = []
      rules[ruleName].append(timeTaken)
      

#print("rules", rules)
for key, value in rules.items():
  print(key, len(value), statistics.mean(value), statistics.median(value), statistics.pstdev(value) )

print("# Finished")
