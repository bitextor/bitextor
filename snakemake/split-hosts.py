#!/usr/bin/env python3
import sys
import os
import tldextract
import argparse
import gzip

###########################################################
def CreateDomainKey2HostMap(hosts):
    ret={}
    for host in hosts:
        # don't merge blog sites
        if host.find(".blogspot.") >= 0 or host.find(".wordpress.") >= 0:
           key = host
        else:
           key = tldextract.extract(host).domain

        if key not in ret:
            ret[key]=[]
        ret[key].append(host)
        #print("subdomain", key, host)
    return ret

###########################################################

oparser = argparse.ArgumentParser(description="Create x number of files with hosts from stdin. Make sure host with same key are in same file")
oparser.add_argument('--num-groups', dest='numGroups', help='Number of groups to create', type=int, required=True)
options = oparser.parse_args()

files = []
for i in range(options.numGroups):
    fileName = "{}.gz".format(i)
    file = gzip.open(fileName, "wt")
    files.append(file)
    
hosts = set()

for host in sys.stdin:
    host = host.strip()
    hosts.add(host)

map = CreateDomainKey2HostMap(hosts)
#print(keys)

i = 0
for key in map:
    #print(key)
    ind = i % options.numGroups

    file = files[ind]
    hosts = map[key]
    #print(hosts)
    for host in hosts:
        #print(host)
        file.write(host + "\n")
        
    i += 1
        
for file in files:
    file.close()
