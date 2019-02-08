#!/usr/bin/env python3

import os
import sys
import argparse

print("Starting")

oparser = argparse.ArgumentParser(description="Hello")
oparser.add_argument('--hosts-file', dest='hostsPath', help='File containing hosts to be excluded', required=True)
oparser.add_argument('--input-dir', dest='inDir', help='Root directory of domains', required=True)
oparser.add_argument('--output-dir', dest='outDir', help='Where to move them to', required=True)
options = oparser.parse_args()

with open(options.hostsPath, 'rt') as f:
  excludedHosts = f.read().splitlines()
  #print("excludedHosts", excludedHosts)
                
for dir in os.listdir(options.inDir):
  if dir in excludedHosts:
    cmd = "    mv {0}/{1} {2}/{1}".format(options.inDir, dir, options.outDir)
    print(cmd)
    
                    
print("Finished")
