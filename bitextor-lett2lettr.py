#!__ENV__ __PYTHON__
# -*- coding: utf-8 -*-

#
# 1. File .lett is processed to obtain a representation of the structure formatted as a string (here called 'raspa'). This 'raspa' provides a simple representation containing HTML tags and text length in the form of a string, which allows easy to apply distance metrics such as Levenshtain's distance.
# 2. The final representation is added to each line as an additional field
#
# Final output format:
# language	encoding	mimetype	url	content(base_64)	raspa
#
#

  # Se decodifica el base64
import sys
import base64
import math
import fileinput
import html.parser
import argparse
import datetime

class Parser(html.parser.HTMLParser):

  def __init__( self ):
    html.parser.HTMLParser.__init__( self )
    self.script = 0
    self.output = []

  def handle_starttag( self, tag, attrs ):
    if tag == "script" or tag == "noscript":
      self.script = 1
    else:
      self.output.append("_" + tag + "_")

  def handle_data( self, data ):
    if self.script == 0:
      if data != "":
        nwords=len(data.split())
        if nwords > 0:
          #Replacing every word in text by a "_" symbol
          self.output.append("_"*int(math.log(nwords, 2)))

  def handle_endtag( self, tag ):
    if tag == "script" or tag == "noscript":
      self.script = 0
    else:
      self.output.append("_" + tag + "_")

oparser = argparse.ArgumentParser(description="Script that obtains an abstract representation of every file in a crawled website. The script reads the output from bitextor-ett2lett, reads the content of each file, and produces the representation, which is later used by bitextor-distancefilter to rank the aligned-document candidates.")
oparser.add_argument('lett', metavar='LETT', nargs='?', help='File produced by bitextor-ett2lett containing information about the files in the website (if undefined, the script will read from the standard input)', default=None)
options = oparser.parse_args()

if options.lett != None:
  reader = open(options.lett,"r")
else:
  reader = sys.stdin

for line in reader:
  content=line.strip().split("\t")
  if len(content) >= 6:
    e = base64.b64decode(content[4]).decode("utf8")
    if e != "":
      p=Parser()
      try:
        p.feed(e)
        raspa = "".join(p.output)
        if raspa.split('_')[1][-2:] == "ml" and all(ord(char) < 128 for char in raspa): #Delete entries without *ml in the first tag to avoid things different than HTML or XML as JPGS or PDF, for example.
          print(line.strip()+"\t"+raspa+"\t"+str(datetime.datetime.now().time()))
      except html.parser.HTMLParseError:
        pass

