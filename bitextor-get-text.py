#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import time
import base64
import html5lib
from lxml import etree
import re
import argparse
import ftfy
from lxml.html.clean import Cleaner
from bs4 import BeautifulSoup

#Inline tags that don't start on a new line and only take up as much width as necessary. From https://www.w3schools.com/html/html_blocks.asp
inline_tags={"a","abbr","acronym","b","bdo","big","br","button","cite","code","dfn","em","i","img","input","kbd","label","map","object","q","samp","script","select","small","span","strong","sub","sup","textarea","time","tt","var"}

def getElementText(element,document):
    """Returns a list with word plain text of a tree element of lxml and the corresponding text
    """
    #Return variables for plain text
    text=""
    if element.text != None:        #If we have text in tag
        #Add interpreted space for non-inline tags
        if element.tag not in inline_tags:
            text = "\n"  #Add artificial separation as browser parser does if tag is not inline
        text = text + element.text
    
    #Now we recursively iterate through the childs
    for child in element:
        if type(child) is etree._Element and child.tag != "script" and child.tag != "style":
            text = text + getElementText(child,document)

    #Add interpreted space for non-inline tags after all processing of the actual tag content
    if element.tag not in inline_tags:
        text = text + "\n"
    elif element.tag == "br":
        text = text + "\n"

    #Processing parent text (A.K.A. element.tail) similarly as the actual tag
    if element.tail != None:        #If we have tail parent text (text after the closing tag until the next open/close tag)
        text = text + element.tail

    return text

def getDocumentText(document):
    """Returns a list with word plain text of a document tree in lxml format
    """
    docplaintext=""
    for element in document.getroot():
        if type(element) is etree._Element and element.tag != "script" and element.tag != "style": #Only elements, not scripts or other kind of tags without proper text
            docplaintext = docplaintext + getElementText(element,document)
    return docplaintext


parser = argparse.ArgumentParser(description='Generates (stdout) Stand-off Annotation of HTML documents given in Bitextor crawl format (stdin)')
parser.add_argument('--root-dir', dest='rootDir', help='Domain directory')

args = parser.parse_args()

#Input (stdin) in Bitextor crawl format:
#mime      encoding      url     html_content(base_64)       timestamp

#Output (stdout):
#mime      encoding      url     html_content(base_64)       timestamp     html_text(base_64)

pageFile = open("{rootDir}/raw-html/page".format(rootDir=args.rootDir), "r")
pages = pageFile.read().strip().split("\n")
pageFile.close()

mimeFile = open("{rootDir}/mime.txt".format(rootDir=args.rootDir), "r")
mimes = mimeFile.read().strip().split("\n")
mimeFile.close()

dedupedFile = open("{rootDir}/deduped".format(rootDir=args.rootDir), "r")
lineNums = dedupedFile.read().strip().split("\n")
dedupedFile.close()

for lineNum in lineNums:
    lineNum = int(lineNum)
    deboiledFile = open("{rootDir}/deboiled/{name}".format(rootDir=args.rootDir, name=lineNum), "r")
    html = deboiledFile.read()
    deboiledFile.close()

    soup = BeautifulSoup(html, "lxml", from_encoding='utf-8')
    for script in soup(["script", "style", "img"]):
        script.extract()    # rip it out

    # get text
    text = soup.get_text()
    text = re.sub(r"\n+","\n",re.sub(r" *\n *","\n",re.sub(r" +"," ",re.sub(r"\r","", text))))
    #sys.stderr.write(text + "\n")

    textFile = open("{rootDir}/text/{name}".format(rootDir=args.rootDir, name=lineNum), "wt")
    textFile.write(text)
    textFile.close()

    pageToks = pages[lineNum].split("\t")
    assert(len(pageToks) == 2)

    mimeToks = mimes[lineNum].split("\t")
    assert(len(mimeToks) == 2)

    outFields = [mimeToks[0],
                 mimeToks[1],
                 pageToks[0],
                 base64.b64encode(html.encode()).decode("utf8"),
                 base64.b64encode(text.encode()).decode("utf8"),
                 str(lineNum) ]
