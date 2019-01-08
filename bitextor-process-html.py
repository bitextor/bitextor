#!/usr/bin/env python3

#
# 1. This script reads a tab-separated list of documents, only containing two fields: the content of the document encoded with base64 and the URL
# 2. It decompress the base64 field and uses libmagic to identify the MIME type and character encoding
# 3. The output produced is:
#    character_encoding     MIME    URL    content_base64
#
import os
import sys
import magic
import base64
import argparse
import re
import html5lib
import ftfy
import pycld2 as cld2
from lxml.html.clean import Cleaner
from bs4 import BeautifulSoup
from lxml import etree
from boilerpipe.extract import Extractor

######################################################################################

# Inline tags that don't start on a new line and only take up as much width as necessary. From https://www.w3schools.com/html/html_blocks.asp
inline_tags = {"a", "abbr", "acronym", "b", "bdo", "big", "br", "button", "cite", "code", "dfn", "em", "i", "img",
               "input", "kbd", "label", "map", "object", "q", "samp", "script", "select", "small", "span", "strong",
               "sub", "sup", "textarea", "time", "tt", "var"}


def getElementText(element, document):
  """Returns a list with word plain text of a tree element of lxml and the corresponding text
  """
  # Return variables for plain text
  text = ""
  if element.text != None:  # If we have text in tag
    # Add interpreted space for non-inline tags
    if element.tag not in inline_tags:
      text = "\n"  # Add artificial separation as browser parser does if tag is not inline
    text = text + element.text

  # Now we recursively iterate through the childs
  for child in element:
    if type(child) is etree._Element and child.tag != "script" and child.tag != "style":
      text = text + getElementText(child, document)

  # Add interpreted space for non-inline tags after all processing of the actual tag content
  if element.tag not in inline_tags:
    text = text + "\n"
  elif element.tag == "br":
    text = text + "\n"

  # Processing parent text (A.K.A. element.tail) similarly as the actual tag
  if element.tail != None:  # If we have tail parent text (text after the closing tag until the next open/close tag)
    text = text + element.tail

  return text


def getDocumentText(document):
  """Returns a list with word plain text of a document tree in lxml format
  """
  docplaintext = ""
  for element in document.getroot():
    if type(
            element) is etree._Element and element.tag != "script" and element.tag != "style":  # Only elements, not scripts or other kind of tags without proper text
      docplaintext = docplaintext + getElementText(element, document)
  return docplaintext

def guess_lang_from_data2(data):
  reliable, text_bytes, detected_languages = cld2.detect(
    data, isPlainText=False)
  #print("detected_languages", detected_languages)
  return detected_languages[0][1]

######################################################################################

oparser = argparse.ArgumentParser(description="Script that takes the output of bitextor-crawl and adds to the list of fields the MIME type and the character encoding detected.")
oparser.add_argument('--input', dest='input', help='Input HTML file')
oparser.add_argument('--metadata', dest='metadata', help='Input file containing the URL and crawling time for the input file')
oparser.add_argument('--normhtml', dest='normhtml', help='Output file with normalised HTML')
oparser.add_argument('--deboiled', dest='deboiled', help='Output file with HTML where boilerplates have been removed')
oparser.add_argument('--text', dest='text', help='Output file with plain text from normalized HTML without boilerplates')
oparser.add_argument('--info', dest='info', help='Output file with information about the file: lang, URL, MIME, etc.')

options = oparser.parse_args()

m=magic.open(magic.MAGIC_NONE)
m.load()
#sys.stderr.write("m:" + str(m) + "\n")

filename=os.path.basename(options.input)

with open("{metadata}".format(metadata=options.metadata), "rt") as metadata:
  #Reading URL and date from metadata file
  fields = metadata.read().strip().split("\t")

  if len(fields)>=1:
    url=fields[0]

    #~Mime and encodign
    m.setflags(16|1024)

    # read file
    file = open(options.input, "r")
    text = file.read()
    file.close()
    #sys.stderr.write("text " + str(type(text)) + "\n")

    # encoding and char set
    mimeEncode = m.buffer(text.encode()).split(" ")
    mimeEncode[0] = mimeEncode[0][:-1]
    #sys.stderr.write("mimeEncode:" + str(mimeEncode) + "\n")

    #magicoutput = mimeEncode
    magicoutput = []
    magicoutput.append(url)
    #sys.stderr.write("magicoutput:" + str(magicoutput) + "\n")

    # normalize html
    cleaner=Cleaner(style=True, links=True, add_nofollow=True,page_structure=False, safe_attrs_only=False)

    cleanhtml = cleaner.clean_html(re.sub(r'encoding *= *"[^"]+"', '', text, flags=re.IGNORECASE))
    document = html5lib.parse(ftfy.fix_text(cleanhtml), treebuilder="lxml", namespaceHTMLElements=False)
    tree = etree.tostring(document)
    cleantree = tree.decode("utf8")
    cleantree = cleantree.replace("\t", " ")

    file = open(options.normhtml, "w")
    file.write(cleantree)
    file.close()

    # remove boilerplate html
    #dir = os.path.dirname(os.path.abspath(__file__))
    #cmd = "java -Dfile.encoding=UTF-8 -jar {BITEXTOR}/piped-boilerpipe/piped-boilerpipe.jar {rootDir}/norm-html/{name} {rootDir}/deboiled/{name}".format(BITEXTOR=dir, rootDir=options.rootDir, name=lineNum)
    #os.system(cmd)
    extractor = Extractor(extractor='ArticleExtractor', html=cleantree)
    extracted_text = extractor.getHTML()
    file = open(options.deboiled, "w")
    file.write(extracted_text)
    file.close()

    # get text
    deboiledFile = open(options.deboiled, "r")
    html = deboiledFile.read()
    deboiledFile.close()

    soup = BeautifulSoup(html, "lxml", from_encoding='utf-8')
    for script in soup(["script", "style", "img"]):
        script.extract()    # rip it out

    text = soup.get_text()
    text = re.sub(r"\n+","\n",re.sub(r" *\n *","\n",re.sub(r" +"," ",re.sub(r"\r","", text))))
    #sys.stderr.write(text + "\n")

    textFile = open(options.text, "wt")
    textFile.write(text)
    textFile.close()

    # lang id
    lang = guess_lang_from_data2(html)

    file = open(options.info, "w")
    file.write(lang + "\t" +  mimeEncode[0] + "\t" + mimeEncode[1] + "\t" + url + "\t" + str(filename).split(".")[0] + "\n")
    file.close()

  else:
    sys.stderr.write("Wrong line: "+line.strip()+"\n")


#with open("{rootDir}/page".format(rootDir=options.rootDir), "wt") as pageFile:
#  pageFile.write("\n".join(pages))
#  pageFile.write("\n")
