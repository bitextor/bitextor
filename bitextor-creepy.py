#!/usr/bin/env python3

# -*- coding: utf-8 -*-
#
# This file has been created building on the crawler.py script
# in the Creepy project, by Wei-Ning Huang (AZ) <aitjcize@gmail.com>
# and is distributed under GPLv3 license.
#
# The script allows to crawl a website. It takes, as the input, a URL
# and a nubmer of optional parameters (use option -h for more details).
# The output is a WARC file

import argparse
import hashlib
import http.client
import logging
import pickle
import random
import re
import signal
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
import urllib.robotparser
import requests
from posixpath import join, dirname, normpath
from ssl import CertificateError
from threading import Thread, Lock
from urllib.parse import quote

import warc


# SET THE SEED FOR REPRODUCIBILITY TESTS
# SEED=4
# random.seed(SEED)


class Document(object):
    def __init__(self, res, url):
        self.url = url
        self.query = '' if '?' not in url else url.split('?')[-1]
        self.status = res.status
        self.text = res.read()

        self.headers = dict(res.getheaders())
        self.links = []


class Crawler(object):
    F_ANY, F_SAME_DOMAIN, F_SAME_HOST, F_SAME_PATH, F_TLD = list(range(5))

    def __init__(self, debug=False):
        self.seencontent = set()
        self.currdomain = ""
        self.visited = {}
        self.outerdomaintargets = {}
        self.targets = []
        self.seen = set()
        self.threads = []
        self.concurrency = 0
        self.max_outstanding = 16
        self.max_depth = 50
        self.root_url = None
        self.proto = None
        self.host = None
        self.path = None
        self.dir_path = None
        self.query = None
        self.crawlstarts = time.time()
        self.crawlsize = 0.0
        self.sizelimit = None
        self.timelimit = None
        self.robotsparser = urllib.robotparser.RobotFileParser()
        self.interrupt = False
        self.timeout = 10
        self.TLdomain = ""
        self.verbose = False
        self.delay = 0
        self.useragent = None

        self.follow_mode = self.F_SAME_HOST
        self.content_type_filter = '(text/html)'
        self.url_filters = []
        self.prefix_filter = '^(#|javascript:|mailto:)'

        self.targets_lock = Lock()
        self.concurrency_lock = Lock()
        self.delay_lock = Lock()

        logging.basicConfig(level=logging.DEBUG if debug else logging.ERROR)

        self.dumpfile = None

    def set_content_type_filter(self, cf):
        self.content_type_filter = '(%s)' % ('|'.join(cf))

    def set_timeout(self, time):
        self.timeout = time

    def add_url_filter(self, uf):
        self.url_filters.append(uf)

    def set_follow_mode(self, mode):
        if mode > 6:
            raise RuntimeError('invalid follow mode.')
        self.follow_mode = mode

    def set_concurrency_level(self, level):
        self.max_outstanding = level

    def set_max_depth(self, max_depth):
        self.max_depth = max_depth

    def process_document(self, doc):
        print('GET', doc.status, doc.url, doc.links)
        # to do stuff with url depth use self._calc_depth(doc.url)

    def keep_crawling(self):
        self.targets_lock.acquire()
        self.targets = list(self.outerdomaintargets[list(self.outerdomaintargets.keys())[0]])
        self.seen = set(self.targets)
        del self.outerdomaintargets[list(self.outerdomaintargets.keys())[0]]
        self.targets_lock.release()
        self.root_url = self.targets.pop()
        self.crawl(self.root_url)

    def init_crawling(self, url):
        self.root_url = url

        self.targets.append(url)
        self.seen.add(url)
        self.crawl(url)

    def crawl(self, url):
        rx = re.match('(https?://)([^/]+)([^\?]*)(\?.*)?', url)
        if rx is None:
            url = "http://" + url
            rx = re.match('(https?://)([^/]+)([^\?]*)(\?.*)?', url)
        self.proto = rx.group(1)
        self.host = rx.group(2)
        self.path = rx.group(3)
        self.dir_path = dirname(self.path)
        self.query = rx.group(4)

        self.TLdomain = self.host.split(".")[-1]
        self.currdomain = self._url_domain(self.host)

        try:
            self.robotsparser.set_url(self.proto + self.host + "/robots.txt")
            self.robotsparser.read()
        except IOError:
            sys.stderr.write(
                "It was not possible to connect to the web server specified to retrieve the robots.txt file")
        except CertificateError:
            sys.stderr.write("Certificate error: ")
            sys.stderr.write(str(sys.exc_info()[0]) + "\n")

        self._spawn_new_worker()
        if self.verbose:
            sys.stderr.write("Starting thread\n")

        while self.threads:
            try:
                for t in self.threads:
                    try:
                        t.join(1)
                        if not t.isAlive():
                            self.threads.remove(t)
                            if self.verbose:
                                sys.stderr.write("Killing thread\n")
                    except RuntimeError:
                        pass

            except KeyboardInterrupt:
                sys.exit(1)

    @staticmethod
    def _url_domain(host):
        parts = host.split('.')
        if len(parts) <= 2:
            return host
        elif re.match('^[0-9]+(?:\.[0-9]+){3}$', host):  # IP
            return host
        else:
            return '.'.join(parts[1:])

    def _follow_link(self, url, link):
        # Longer than limit set by the standard RFC7230 are discarded
        if len(link) > 2000:
            return None

        # Remove anchor
        link = re.sub(r'#[^#]*$', '', link)

        # Skip prefix
        if re.search(self.prefix_filter, link):
            return None

        # Filter url
        for f in self.url_filters:
            if re.search(f, link):
                return None

        rx = re.match('(https?://)([^/:]+)(:[0-9]+)?([^\?]*)(\?.*)?', url)
        url_proto = rx.group(1)
        url_host = rx.group(2)
        url_port = rx.group(3) if rx.group(3) else ''
        url_path = rx.group(4) if len(rx.group(4)) > 0 else '/'
        url_dir_path = dirname(url_path)

        rx = re.match('((https?://)([^/:]+)(:[0-9]+)?)?([^\?]*)(\?.*)?', link)
        link_full_url = rx.group(1) != None
        link_proto = rx.group(2) if rx.group(2) else url_proto
        link_host = rx.group(3) if rx.group(3) else url_host
        link_port = rx.group(4) if rx.group(4) else url_port
        link_path = rx.group(5) if rx.group(5) else url_path
        link_query = quote(rx.group(6), '?=&%/') if rx.group(6) else ''

        if not link_full_url and not link.startswith('/'):
            link_path = normpath(join(url_dir_path, link_path))
        link_dir_path = dirname(link_path)

        link_url = link_proto + link_host + link_port + link_path + link_query
        if self.follow_mode == self.F_ANY:
            return link_url
        elif self.follow_mode == self.F_TLD:
            dom = self._url_domain(link_host)
            if dom == self.currdomain:
                return link_url
            elif dom.split(".")[-1] == self.TLdomain:
                self.targets_lock.acquire()
                if dom not in self.outerdomaintargets:
                    self.outerdomaintargets[dom] = set()
                self.outerdomaintargets[dom].add(link_url)
                self.targets_lock.release()
                sys.stderr.write("'" + link + "' stored in the list of domains\n")
                return None
            else:
                sys.stderr.write("'" + link + "' discarded: not in the same TLD\n")
                return None
        elif self.follow_mode == self.F_SAME_DOMAIN:
            return link_url if self._url_domain(self.host) == \
                               self._url_domain(link_host) else None
        elif self.follow_mode == self.F_SAME_HOST:
            return link_url if self.host == link_host else None
        elif self.follow_mode == self.F_SAME_PATH:
            if self.host == link_host and \
                    link_dir_path.startswith(self.dir_path):
                return link_url
            else:
                return None

    def _calc_depth(self, url):
        # calculate url depth
        return len(url.replace('https', 'http').replace(self.root_url, '')
                   .rstrip('/').split('/')) - 1

    def _add_target(self, target):
        if not target:
            return

        if self.max_depth and self._calc_depth(target) > self.max_depth:
            return

        self.targets_lock.acquire()
        if target in self.visited:
            self.targets_lock.release()
            return
        if target not in self.seen:
            self.targets.append(target)
            self.seen.add(target)
        self.targets_lock.release()

    def _spawn_new_worker(self):
        self.concurrency_lock.acquire()
        try:
            self.concurrency += 1
            t = Thread(target=self._worker, args=(self.concurrency,))
            t.daemon = True
            self.threads.append(t)
            t.start()
        finally:
            self.concurrency_lock.release()

    def _worker(self, _):
        while self.targets:
            if self.interrupt:
                break
            else:
                url = None
                self.targets_lock.acquire()
                if len(self.targets) > 0:
                    url = self.targets.pop()
                    if url not in self.visited:
                        self.visited[url] = 1
                self.targets_lock.release()

                if url is not None:
                    try:
                        logging.debug('url: %s' % url)

                        if not self.robotsparser.can_fetch("*", url):
                            sys.stderr.write("robots.txt forbids crawling URL: " + url + "\n")
                        else:
                            if self.verbose:
                                sys.stderr.write("Crawling URL: " + url + "\n")

                            rx = re.match('(https?)://([^/]+)(.*)', url)
                            protocol = rx.group(1)
                            host = rx.group(2)
                            path = rx.group(3)

                            # Connections are done with a delay to avoid blocking the server
                            self.delay_lock.acquire()
                            if protocol == 'http':
                                conn = http.client.HTTPConnection(host, timeout=self.timeout)
                            else:
                                conn = http.client.HTTPSConnection(host, timeout=self.timeout)

                            if self.useragent == None:
                                conn.request('GET', quote(path, '?=&%/'))
                            else:
                                conn.request('GET', quote(path, '?=&%/'), headers={'User-Agent': self.useragent})

                            res = conn.getresponse()

                            if 301 <= res.status <= 308:
                                rlink = self._follow_link(url, res.getheader('location'))
                                self._add_target(rlink)
                                logging.info('redirect: %s -> %s' % (url, rlink))
                                conn.close()
                                time.sleep(self.delay)
                                self.delay_lock.release()
                                continue

                            # Check content type
                            try:
                                if not re.search(self.content_type_filter,
                                                 res.getheader('Content-Type')):
                                    #sys.stderr.write(url + " discarded: wrong file type\n")
                                    conn.close()
                                    time.sleep(self.delay)
                                    self.delay_lock.release()
                                    continue
                            except TypeError:  # getheader result is None
                                conn.close()
                                time.sleep(self.delay)
                                self.delay_lock.release()
                                continue

                            doc = Document(res, url)
                            conn.close()
                            time.sleep(self.delay)
                            self.delay_lock.release()

                            c = hashlib.md5()
                            c.update(re.sub(rb"href\s*=\s*['\"]\s*([^'\"]+)['\"]", b"", doc.text))
                            if c.hexdigest() not in self.seencontent:
                                # Make unique list (these are the links in the document)
                                try:
                                    links = re.findall("href\s*=\s*['\"]\s*([^'\"]+)['\"]", doc.text.decode('utf8'))
                                    # content = re.sub('<atom:link[^>]*>', '', doc.text.decode('utf8'))
                                except:
                                    links = re.findall("href\s*=\s*['\"]\s*([^'\"]+)['\"]", doc.text.decode('latin1'))

                                    # content = re.sub('<atom:link[^>]*>', '', doc.text.decode('latin1'))

                                    # sys.stderr.write(str(content)+"\n")

                                    # content = re.sub('<head>.*</head>', '', content)
                                    # links = re.findall("href\s*=\s*['\"]\s*([^'\"]+)['\"]",
                                    #      content)

                                linksset = list(set(links))
                                random.shuffle(linksset)
                                self.process_document(doc)

                                for link in linksset:
                                    rlink = self._follow_link(url, link.strip())
                                    self._add_target(rlink)

                            if self.concurrency < self.max_outstanding:
                                if self.verbose:
                                    sys.stderr.write("Starting thread\n")
                                self._spawn_new_worker()
                    except KeyError as e:
                        # Pop from an empty set
                        break

                    except (http.client.HTTPException, EnvironmentError) as e:
                        time.sleep(self.delay)
                        self.delay_lock.release()

                        if self.sizelimit is not None and self.crawlsize > self.sizelimit:
                            self.concurrency_lock.acquire()
                            self.interrupt = True
                            self.concurrency_lock.release()
                        elif self.timelimit is not None and time.time() - self.crawlstarts > self.timelimit:
                            self.concurrency_lock.acquire()
                            self.interrupt = True
                            self.concurrency_lock.release()
                        else:
                            self.targets_lock.acquire()
                            if self.visited[url] <= 5:
                                logging.error('%s: %s, retrying (attempt %s)' % (url, str(e), str(self.visited[url])))
                                self.targets.append(url)
                                self.visited[url] = self.visited[url] + 1
                                self.seen.add(url)
                            else:
                                logging.error('%s: %s, given up after 5 attempts' % (url, str(e)))
                            self.targets_lock.release()
            if crawler.timelimit is not None and time.time() - crawler.crawlstarts > crawler.timelimit:
                self.interrupt = True

        self.concurrency_lock.acquire()
        self.concurrency -= 1
        self.concurrency_lock.release()


oparser = argparse.ArgumentParser(
    description="Script that crawls a website and prints the downloaded documents"
                "in standard output using WARC format.")
oparser.add_argument("URL", metavar="FILE", nargs="?", help="URL of the website to be downloaded", default=None)
oparser.add_argument("-t", help="Time limit after which crawling will be stopped", dest="timelimit", required=False,
                     default=None)
oparser.add_argument("-s", help="Total size limit; once it is reached the crawling will be stopped", dest="sizelimit",
                     required=False, default=None)
oparser.add_argument("-j", help="Number of crawling jobs that can be run in parallel (threads)", dest="jobs",
                     required=False, default=8, type=int)
oparser.add_argument("-o", help="Timeout limit for a connexion in seconds", dest="timeout", required=False, default=8,
                     type=int)
oparser.add_argument("-d", help="Dump crawling status if program is stopped by SIGTERM", dest="dump", required=False,
                     default=None)
oparser.add_argument("-T", "--wait", help="Time delay between requests in seconds; by default it is set to 2s", dest="delay",
                     required=False, default=2, type=int)
oparser.add_argument("-l", help="Continue an interrupted crawling. Load crawling status from this file", dest="load",
                     required=False, default=None)
oparser.add_argument("-e", help="Continue an interrupted crawling. Load ETT from this file", dest="resumeett",
                     required=False, default=None)
oparser.add_argument("-a", help="User agent to be added to the headers of the requests", dest="agent",
                     required=False, default=None)
oparser.add_argument("-D",
                     help="This option allows to run Bitextor on a mode that crawls a TLD starting from the URL "
                          "provided.",
                     dest="crawltld", action='store_true')
oparser.add_argument("-v", help="Verbose mode.", dest="verbose", action='store_true')
options = oparser.parse_args()

rp = urllib.robotparser.RobotFileParser()
if '//' not in options.URL:
    options.URL = '%s%s' % ('http://', options.URL)
robots = requests.get(options.URL+"/robots.txt").text.split("\n")
for line in robots:
    if "Crawl-delay" in line:
        crawldelay=int(line.split(':')[1].strip())
        if options.delay is None or crawldelay > int(options.delay):
            options.delay = str(crawldelay)

class MyCrawler(Crawler):
    def process_document(self, doc):
        if doc.status == 200:
            self.concurrency_lock.acquire()
            try:
                # print base64.b64encode(doc.text)+"\t"+doc.url+"\t"+str(time.time())
                warc_record = warc.WARCRecord(payload=doc.text, headers={"WARC-Target-URI": doc.url})
                f = warc.WARCFile(fileobj=sys.stdout.buffer)
                f.write_record(warc_record)
                self.crawlsize += sys.getsizeof(doc.text) / 1000000.0
                if self.sizelimit is not None and self.crawlsize > self.sizelimit:
                    self.interrupt = True
                    self.save_status()
                if self.timelimit is not None and time.time() - self.crawlstarts > self.timelimit:
                    self.interrupt = True
                    self.save_status()
            finally:
                self.concurrency_lock.release()
        else:
            pass

    def get_status_object(self):
        return {'visited': self.visited, 'targets': self.targets, 'seen': self.seen}

    def load_status(self, statusobj):
        self.visited = statusobj['visited']
        self.targets = statusobj['targets']
        self.seen = statusobj['seen']

    def save_status(self):
        if self.dumpfile is not None:
            sys.stderr.write("Saving crawling status to " + self.dumpfile + "\n")
            pickle.dump(self.get_status_object(), open(self.dumpfile, 'wb'))

    def termsighandler(self, signum, frame):
        sys.stderr.write("Stopping crawling by user's SIGTERM\n")
        sys.stdout.flush()
        sys.stderr.flush()
        self.concurrency_lock.acquire()
        self.interrupt = True
        self.save_status()
        self.concurrency_lock.release()
        # exit(143) #128 + 15(SIGTERM).


if not options.URL.startswith("http"):
    if options.URL.find("://") != -1:
        sys.stderr.write("Error: '" + options.URL.split("://")[
            0] + "' is not a valid protocol; you should use either 'http' or 'https'\n")
        sys.exit(-1)
    else:
        sys.stderr.write("No protocol provided in URL -- assuming http\n")
        options.URL = "http://" + options.URL

crawler = MyCrawler()
crawler.verbose = options.verbose
crawler.set_concurrency_level(options.jobs)
crawler.delay = options.delay
crawler.useragent = options.agent
if options.crawltld:
    crawler.set_follow_mode(Crawler.F_TLD)
else:
    crawler.set_follow_mode(Crawler.F_SAME_DOMAIN)
crawler.set_timeout(20)
if options.sizelimit is not None:
    unit = options.sizelimit[-1]
    if unit == 'G':
        crawler.sizelimit = float(options.sizelimit[:-1]) * 1000
    elif unit == 'M':
        crawler.sizelimit = float(options.sizelimit[:-1])
    elif unit == 'K':
        crawler.sizelimit = float(options.sizelimit[:-1]) / 1000.0
    else:
        sys.stderr.write(
            "The value of option -s (download size limit) has to be a number and a unit ('G' for gigabytes, "
            "'M' for megabytes, or 'K' for kilobytes), for example: 10M or 150K\n")
        sys.exit(-1)

if options.timelimit is not None:
    unit = options.timelimit[-1]
    if unit == 'h':
        crawler.timelimit = float(options.timelimit[:-1]) * 3600
    elif unit == 'm':
        crawler.timelimit = float(options.timelimit[:-1]) * 60
    elif unit == 's':
        crawler.timelimit = float(options.timelimit[:-1])
    else:
        sys.stderr.write(
            "The value of option -t (download time limit) has to be a number and a unit ('s' for second, "
            "'m' for minutes, or 'h' for hours), for example: 15m or 3h\n")
        sys.exit(-1)

if options.dump is not None:
    crawler.dumpfile = options.dump

if options.load is not None:
    sys.stderr.write("Restoring crawling from " + options.load + "\n")
    crawler.load_status(pickle.load(open(options.load, 'rb')))
if options.resumeett is not None:
    for line in open(options.resumeett):
        print(line.rstrip("\n"))

# crawler.add_url_filter('\.(jpg|jpeg|gif|png|js|css|swf)$')
signal.signal(signal.SIGTERM, crawler.termsighandler)
crawler.init_crawling(options.URL)
if options.crawltld:
    while len(list(crawler.outerdomaintargets.keys())) > 0:
        sys.stderr.write("Remaining " + str(len(list(crawler.outerdomaintargets.keys()))) + " websites to to crawl\n")
        crawler.keep_crawling()

if crawler.interrupt:
    if crawler.sizelimit is not None and crawler.crawlsize > crawler.sizelimit:
        sys.stderr.write("Crawling size limit reached: stopping crawl\n")
    elif crawler.timelimit is not None and time.time() - crawler.crawlstarts > crawler.timelimit:
        sys.stderr.write("Crawling time limit reached: stopping crawl\n")
