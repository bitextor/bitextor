#!/usr/bin/env python3

import os
import sys
import argparse
import tldextract

################################################################
def GetDomainKeys(hosts):
    keys = set()
    for host in hosts:
        domain = tldextract.extract(host).domain
        keys.add(domain)
    return keys
################################################################

print("Starting")

oparser = argparse.ArgumentParser(description="Hello")
oparser.add_argument('--hosts-file', dest='hostsPath', help='File containing hosts to be excluded', required=True)
oparser.add_argument('--input-dir', dest='inDir', help='Root directory of domains', required=True)
oparser.add_argument('--output-dir', dest='outDir', help='Where to move them to', required=True)
options = oparser.parse_args()

with open(options.hostsPath, 'rt') as f:
  excludeHosts = f.read().splitlines()
  print("excludeHosts", len(excludeHosts))

  excludeKeys = GetDomainKeys(excludeHosts)
  print("excludeKeys", len(excludeKeys))
  #print("excludeKeys", excludeKeys)

for dir in os.listdir(options.inDir):
  key = tldextract.extract(dir).domain
  if key in excludeKeys:
    cmd = "    mv {0}/{1} {2}/{1}".format(options.inDir, dir, options.outDir)
    print(cmd)
    
                    
print("Finished")
