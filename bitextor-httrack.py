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

import argparse
import os
import subprocess
import sys
import requests
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
    agentoption = ""
    if agent is not None:
        agentoption = "-F \""+agent+"\""

    domain = tldextract.extract(url).domain+"."+tldextract.extract(url).suffix

    cmd += " {URL} --robots=3 --sockets=2 --keep-alive --urlhack -I0 --timeout=30 --host-control=3 --retries=3 -m -O {DOWNLOAD_PATH} {AGENT}  ".format(URL=url, DOWNLOAD_PATH=outPath, AGENT=agentoption, DOMAIN=domain)
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
    try:
        robots = requests.get(args.url+"/robots.txt").text.split("\n")
        for line in robots:
            if "Crawl-delay" in line:
                try:
                    crawldelay = int(line.split(':')[1].strip())
                    if args.wait is None or crawldelay > int(args.wait):
                        args.wait = str(crawldelay)
                except ValueError:
                    continue
    except CertificateError:
        sys.stderr.write("Certificate error: ")
        sys.stderr.write(str(sys.exc_info()[0]) + "\n")
    run(args.url, args.outPath, args.timeLimit, args.pageLimit, args.agent, args.wait)

    print("Finished!")
