#!/usr/bin/env python3

import glob
import argparse
import gzip
import json
import lzma
import os
import random
import shutil
import subprocess
import sys
import tempfile
import tldextract
import traceback
import urllib.parse
from collections import defaultdict
from multiprocessing import Pool
from pathlib import Path

import requests
from tqdm import tqdm

import tldextract

sys.path.append("{0}/..".format(os.path.dirname(os.path.realpath(__file__))))
scriptDir = os.path.dirname(os.path.realpath(sys.argv[0]))


def system_check(cmd):
    sys.stderr.write("Executing:" + cmd + "\n")
    sys.stderr.flush()

    subprocess.check_call(cmd, shell=True)


def run(url, outPath, timeLimit, pageLimit, agent, wait):
    cmd = "httrack --skeleton -Q -q -%i0 -u2 -a "

    if timeLimit:
        cmd += " -E{}".format(timeLimit)

    if pageLimit:
        cmd += " -#L{}".format(pageLimit)

    if wait:
        cmd += " --connection-per-second={}".format(1/int(wait))
    agentoption=""
    if agent != None:
        agentoption="-F \""+agent+"\""

    domain = tldextract.extract(url).domain+"."+tldextract.extract(url).suffix

    cmd += " {URL} --robots=3 --sockets=2 --keep-alive --urlhack -I0 --timeout=30 --host-control=3 --retries=3 --extended-parsing yes -m -O {DOWNLOAD_PATH} {AGENT}  ".format(URL=url, DOWNLOAD_PATH=outPath, AGENT=agentoption, DOMAIN=domain)
    # print("cmd", cmd)

    system_check(cmd)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Run httrack.')

    parser.add_argument('--url', dest='url',
                        help='Domain to crawl', required=True)
    parser.add_argument('--output-path', dest='outPath',
                        help='Directory to write to', required=True)
    parser.add_argument('-t', dest='timeLimit',
                        help='Maximum time to crawl.', required=False)
    parser.add_argument('-p', dest='pageLimit',
                        help='Maximum number of pages to crawl.', required=False)
    parser.add_argument('-a', dest='agent',
                        help='User agent to be included in the crawler requests.', required=False, default=None)
    parser.add_argument('--wait', dest='wait',
                        help='Wait N seconds between queries', required=False, default=None)
    args = parser.parse_args()

    print("Starting...")
    if '//' not in args.url:
        args.url = '%s%s' % ('http://', args.url)
    robots = requests.get(args.url+"/robots.txt").text.split("\n")
    for line in robots:
        if "Crawl-delay" in line:
            crawldelay=int(line.split(':')[1].strip())
            if args.wait is None or crawldelay > int(args.wait):
                args.wait = str(crawldelay)

    run(args.url, args.outPath, args.timeLimit, args.pageLimit, args.agent, args.wait)

    print("Finished!")
