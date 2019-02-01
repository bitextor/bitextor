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

###########################################################################################################################
def systemCheck(cmd):
    sys.stderr.write("Executing:" + cmd + "\n")
    sys.stderr.flush()

    subprocess.check_call(cmd, shell=True)

def Run(url, outPath, timeLimit, pageLimit):
    cmd = "httrack --skeleton -Q -q -%i0 -I0 -u2 "

    if timeLimit:
        cmd += " -E{}".format(timeLimit)

    if pageLimit:
        cmd += " -#L{}".format(pageLimit)

    cmd += " {URL} -O {DOWNLOAD_PATH}".format(URL=url, DOWNLOAD_PATH=outPath)
    #print("cmd", cmd)

    systemCheck(cmd)

###########################################################################################################################

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

    args = parser.parse_args()

    print("Starting...")

    Run(args.url, args.outPath, args.timeLimit, args.pageLimit)


    print("Finished!")
