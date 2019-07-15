#!/usr/bin/env python3

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
