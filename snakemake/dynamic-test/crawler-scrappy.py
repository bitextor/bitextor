#!/usr/bin/env python3
# https://www.makeuseof.com/tag/build-basic-web-crawler-pull-information-website-2/

import os
import scrapy
import logging

logging.getLogger('scrapy').setLevel(logging.WARNING)

###############################################################################
class spider1(scrapy.Spider):
    name = 'Wikipedia'
    start_urls = ['https://en.wikipedia.org/wiki/Battery_(electricity)']

    def parse(self, response):
        print("response", response.url)
        #print("response", response.body)
        #print(response.css('h1#firstHeading::text').extract())

