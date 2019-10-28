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
