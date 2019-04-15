#!/usr/bin/env python3
import sys
import os
import tldextract

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

hosts = set()

for host in sys.stdin:
    host = host.strip()
    hosts.add(host)

map = CreateDomainKey2HostMap(hosts)
#print(keys)

for key in map:
    #print(key)
    hosts = map[key]
    #print(hosts)
    for host in hosts:
        print(host)
