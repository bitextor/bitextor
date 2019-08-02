#!/usr/bin/env python3

import argparse
import subprocess
import sys
import os
import requests
from warcio.archiveiterator import ArchiveIterator
from warcio.warcwriter import WARCWriter


def system_check(cmd):
    sys.stderr.write("Executing:" + cmd + "\n")
    sys.stderr.flush()

    subprocess.check_call(cmd, shell=True)

def check_wget_compression(cmd):
    try:
        subprocess.check_call(cmd, shell=True)
        return True
    except:
        return False

def run(url, outPath, timeLimit, agent, filetypes, warcfilename, wait):
    cmd = ""
    if timeLimit:
        cmd += "timeout {} ".format(timeLimit)
    waitoption = ""
    if wait is not None:
        waitoption = "--wait " + wait
    agentoption = ""
    if agent is not None:
        agentoption = "--user-agent \"" + agent + "\""

    filetypesoption = ""
    if filetypes is not None:
        filetypesoption = "-A \"" + filetypes + "\""

    warcoption = ""
    warcfilebasename = warcfilename[0:warcfilename.find(".warc.gz")]
    if warcfilename is not None:
        warcoption = "--warc-file \"" + warcfilebasename + "\""


    if check_wget_compression("wget --help | grep 'no-warc-compression'"):
        warcoption += " --no-warc-compression"

    cmd += "wget --mirror {WAIT} {FILETYPES} -q {URL} -P {DOWNLOAD_PATH} {AGENT} {WARC}".format(WAIT=waitoption,
                                                                                                 FILETYPES=filetypesoption,
                                                                                                 URL=url,
                                                                                                 DOWNLOAD_PATH=outPath,
                                                                                                 AGENT=agentoption,
                                                                                                 WARC=warcoption)
    # print("cmd", cmd)
    try:
        system_check(cmd)
        with open(warcfilebasename + ".warc", 'rb') as f_in:
            with open(warcfilebasename + ".warc.gz", 'wb') as f_out:
                writer = WARCWriter(f_out, gzip=True)
                for record in ArchiveIterator(f_in):
                    writer.write_record(record)
    except subprocess.CalledProcessError as grepexc:
        with open(warcfilebasename + ".warc", 'rb') as f_in:
            with open(warcfilebasename + ".warc.gz", 'wb') as f_out:
                writer = WARCWriter(f_out, gzip=True)
                for record in ArchiveIterator(f_in):
                    writer.write_record(record)
                # try except here

        sys.stderr.write("Warning: Some files could not be downloaded with wget\n")

    system_check("rm {WARC}".format(WARC=warcfilebasename+".warc"))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Run wget.')

    parser.add_argument('--url', dest='url',
                        help='Domain to crawl', required=True)
    parser.add_argument('--output-path', dest='outPath',
                        help='Directory to write to', required=True)
    parser.add_argument('-t', dest='timeLimit',
                        help='Maximum time to crawl.', required=False)
    parser.add_argument('-a', dest='agent',
                        help='User agent to be included in the crawler requests.', required=False, default=None)
    parser.add_argument('-f', dest='filetypes',
                        help='File types to be downloaded, comma separated. For example, "html,pdf"', required=False,
                        default=None)
    parser.add_argument('--warc', dest='warcfilename',
                        help='Write output to a WARC file', required=False, default=None)
    parser.add_argument('--wait', dest='wait',
                        help='Wait N seconds between queries', required=False, default=None)

    args = parser.parse_args()

    print("Starting...")
    if '//' not in args.url:
        args.url = '%s%s' % ('http://', args.url)
    robots = requests.get(args.url + "/robots.txt").text.split("\n")
    for line in robots:
        if "Crawl-delay" in line:
            crawldelay = int(line.split(':')[1].strip())
            if args.wait is None or crawldelay > int(args.wait):
                args.wait = str(crawldelay)
    run(args.url, args.outPath, args.timeLimit, args.agent, args.filetypes, args.warcfilename, args.wait)

    print("Finished!")
