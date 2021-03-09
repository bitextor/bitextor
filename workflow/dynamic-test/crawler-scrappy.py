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
        # print("response", response.body)
        # print(response.css('h1#firstHeading::text').extract())
