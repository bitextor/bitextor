#!/usr/bin/env python3

import argparse
import subprocess
import sys


def system_check(cmd):
    sys.stderr.write("Executing:" + cmd + "\n")
    sys.stderr.flush()

    subprocess.check_call(cmd, shell=True)


def run(url, outPath, timeLimit, agent, filetypes):
    cmd = ""
    if timeLimit:
        cmd += "timeout {} ".format(timeLimit)

    agentoption=""
    if agent != None:
        agentoption="--user-agent \""+agent+"\""
    
    filetypesoption=""
    if filetypes != None:
        filetypesoption="-A \""+filetypes+"\""

    cmd += "wget --mirror --random-wait {FILETYPES} -q {URL} -P {DOWNLOAD_PATH} {AGENT}  ".format(FILETYPES=filetypesoption,URL=url, DOWNLOAD_PATH=outPath, AGENT=agentoption)
    # print("cmd", cmd)
    try:
        system_check(cmd)
    except subprocess.CalledProcessError:
        sys.stderr.write("Warning: Some files could not be downloaded with wget\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Run httrack.')

    parser.add_argument('--url', dest='url',
                        help='Domain to crawl', required=True)
    parser.add_argument('--output-path', dest='outPath',
                        help='Directory to write to', required=True)
    parser.add_argument('-t', dest='timeLimit',
                        help='Maximum time to crawl.', required=False)
    parser.add_argument('-a', dest='agent',
                        help='User agent to be included in the crawler requests.', required=False, default=None)
    parser.add_argument('-f', dest='filetypes',
                        help='File types to be downloaded, comma separated. For example, "html,pdf"', required=False, default=None)

    args = parser.parse_args()

    print("Starting...")

    run(args.url, args.outPath, args.timeLimit, args.agent, args.filetypes)

    print("Finished!")
