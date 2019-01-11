#!/usr/bin/env python3

import warc
import base64
import sys
import argparse
import cchardet
import hashlib
import os
import magic
import re
import html5lib
import ftfy
import pycld2 as cld2
from lxml.html.clean import Cleaner
from bs4 import BeautifulSoup
from lxml import etree
from boilerpipe.extract import Extractor
import alcazar.bodytext
import logging
from common import open_xz_or_gzip_or_plain
import lzma


######################################################################################

def guess_lang_from_data2(data):
  reliable, text_bytes, detected_languages = cld2.detect(
    data, isPlainText=False)
  return detected_languages[0][1]

def convert_encoding(data):
  encoding = cchardet.detect(data)['encoding']

  if len(data) > 0:
    #We convert, even if the text is detected to be UTF8 so, if it is an error and conversion fails, the error is catched here
    for enc in [encoding, 'utf-8', 'iso-8859-1', 'windows‑1252']:
      try:
        return (enc,data.decode(enc))
      except UnicodeDecodeError:
        pass

  return (None,'')

#############################################################################

oparser = argparse.ArgumentParser(description="Script that takes every record in a WARC file and runs preprocessing, which includes: HTML normalization, deduplication, MIME and language identification, and boilerplate removing.")
oparser.add_argument("--verbose", action="store_true", default=False, help="Produce additional information about preprocessing through stderr.")
oparser.add_argument("--boilerpipe", action="store_true", default=False, help="Use boilerpipe bodytext to do the de-boiling")
oparser.add_argument("--alcazar", action="store_true", default=False, help="Use alcazar bodytext extract relevant text from HTML. By default BeautifulSoup4is used")
oparser.add_argument('--output-dir', dest='outDir', help='Output directory', required=True)
oparser.add_argument('--lang1', dest='l1', help='Language l1 in the crawl', default=None)
oparser.add_argument('--lang2', dest='l2', help='Language l2 in the crawl', default=None)
options = oparser.parse_args()

logging.basicConfig(level=logging.INFO if options.verbose else logging.ERROR)

f = warc.WARCFile(fileobj=sys.stdin.buffer)

seen_md5={}
magic.Magic(mime=True)

languages=[]
if options.l1 != None:
  languages.append(options.l1)
if options.l2 != None:
  languages.append(options.l2)

urlFile = lzma.open(options.outDir+"/url.xz", "w")
langFile = lzma.open(options.outDir+"/lang.xz", "w")
encodingFile = lzma.open(options.outDir+"/encoding.xz", "w")
mimeFile = lzma.open(options.outDir+"/mime.xz", "w")
normHtmlFile = lzma.open(options.outDir+"/normalized_html.xz", "w")
plainTextFile = lzma.open(options.outDir+"/plain_text.xz", "w")
#Boilerpipe cleaning is optional
if options.boilerpipe:
  deboilFile = lzma.open(options.outDir+"/deboilerplate_html.xz", "w")

for record in f:
    #We convert into UTF8 first of all
    orig_encoding,text = convert_encoding(record.payload.read())
    url=record.url

    if orig_encoding == None:
      logging.info("Encoding of document "+url+" could not be identified")

    if len(text) > 0:
      #HTML is then normalized
      cleaner=Cleaner(style=True, links=True, add_nofollow=True,page_structure=False, safe_attrs_only=False)
  
      cleanhtml = cleaner.clean_html(re.sub('encoding *= *"[^"]+"', '', text, flags=re.IGNORECASE))
      document = html5lib.parse(ftfy.fix_text(cleanhtml), treebuilder="lxml", namespaceHTMLElements=False)
      tree = etree.tostring(document)
      cleantree = tree.decode("utf8")
      cleantree = cleantree.replace("\t", " ")
  
      # lang id
      lang = guess_lang_from_data2(cleantree)
      if len(languages)>0 and lang not in languages:
        logging.info("Language of document "+url+": "+lang+". Not among searched languages.")
      else:
        #If enabled, remove boilerplate HTML
        if options.boilerpipe:
          extractor = Extractor(extractor='ArticleExtractor', html=cleantree)
          deboiled = extractor.getHTML()
        else:
          deboiled = cleantree

        #We compute MD5 on the HTML (either normalized one or after boilerpipe if enabled): if we get duplicate files we discard them
        c = hashlib.md5()
        c.update(deboiled.encode())

        #checking for duplicate content (duplicates are discarded)
        if c.hexdigest() in seen_md5:
          logging.info("Repeated file:\t"+url+"\tfirst occurrence\t"+seen_md5[c.hexdigest()])
          pass
        else:
          #If enabled get text with Alcazar library
          if options.alcazar:
            btext = alcazar.bodytext.parse_article(cleantree)
            if btext.body_text:
              plaintext = btext.body_text
            else:
              plaintext = ""
          #Otherwise use beautifulsoup
          else:
            if options.boilerpipe:
              soup = BeautifulSoup(deboiled, "lxml")
            else:
              soup = BeautifulSoup(cleantree, "lxml")
            for script in soup(["script", "style", "img"]):
                script.extract()    # rip it out

            plaintext = soup.get_text()
            plaintext = re.sub(r"\n+","\n",re.sub(r" *\n *","\n",re.sub(r" +"," ",re.sub(r"\r","", plaintext))))

          if len(plaintext) > 0:
            #Guessing MIME of the file (checked on original content)
            mime=magic.from_buffer(text, mime=True)
            mimeFile.write(mime.encode()+b"\n")

            urlFile.write(url.encode()+b"\n")
            langFile.write(lang.encode()+b"\n")
            encodingFile.write(orig_encoding.encode()+b"\n")

            b64norm=base64.b64encode(cleantree.encode())
            normHtmlFile.write(b64norm+b"\n")

            if options.boilerpipe:
              b64deboil=base64.b64encode(deboiled.encode())
              deboilFile.write(b64deboil+b"\n")

            b64text=base64.b64encode(plaintext.encode())
            plainTextFile.write(b64text+b"\n")
            #print("{0}\t{1}\t{2}\t{3}\t{4}".format(lang, orig_encoding, mime, b64norm.decode("utf-8"), b64text.decode("utf-8")))

urlFile.close()
langFile.close()
encodingFile.close()
mimeFile.close()
normHtmlFile.close()
plainTextFile.close()
#Boilerpipe cleaning is optional
if options.boilerpipe:
  deboilFile.close()
