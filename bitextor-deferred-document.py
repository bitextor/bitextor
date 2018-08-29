#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import time
import hashlib
import base64
import html5lib
from lxml import etree
import re

def getWordStandoff(element,document,positionsdict):
    """Returns a list with word standoff annotations of a tree element of lxml
    
    Returns a semicolon separated list of word standoffs from a XML/XHTML element. Updates the consumed positions of his parent if tail is found
    """
    global spaceEndPreviousTag
    standoff=[]                
    wordposition=0      #Pointer of actual start word character inside the element tag
    wordpositionparent=positionsdict[element.getparent()]       #Pointer of actual start word character outside the element tag (tail), which belongs to the parent tag
    firstword = True
    if element.text != None:        #If we have text in tag
        for word in re.split(r'(\s+)', element.text): #Split using spaces, but keeping those spaces as words (to count double spaces positions)
            if word.isspace():
                wordposition=wordposition+len(word)
                firstword=False
                spaceEndPreviousTag = True
            elif word == "":
                continue
            else:
                plusprevious = ""
                if firstword and not spaceEndPreviousTag:
                    plusprevious = "+"
                #For every space separated word inside the tag, create a standoff annotation with the path from root, remove the w3 note from tag and the initial and final character positions. Then, update the position pointer
                standoff.append(plusprevious + document.getelementpath(element)+':'+str(wordposition)+"-"+str(wordposition+len(word)-1))
                wordposition=wordposition+len(word)
                spaceEndPreviousTag = False
                firstword=False
    positionsdict[element]=wordposition     #Insert the element pointer in the pointer dictionary (for future children with tail text)
    
    #Now we recursively iterate through the childs
    for child in element:
        if type(child) is etree._Element:
            standoff = standoff + getWordStandoff(child,document,positionsdict)
    
    firstword = True
    if element.tail != None:        #If we have tail parent text (text after the closing tag until the next open/close tag)
        for word in re.split(r'(\s+)', element.tail):
            if word.isspace():
                wordpositionparent=wordpositionparent+len(word)
                firstword=False
                spaceEndPreviousTag = True
            elif word == "":
                continue
            else:
                plusprevious = ""
                if firstword and not spaceEndPreviousTag:
                    plusprevious = "+"
                #Generate word standoff annotation in the same way as the text inside the tag
                standoff.append(plusprevious + document.getelementpath(element.getparent())+':'+str(wordpositionparent)+"-"+str(wordpositionparent+len(word)-1))
                wordpositionparent=wordpositionparent+len(word)
                spaceEndPreviousTag = False
                firstword=False
        positionsdict[element.getparent()]=wordpositionparent       #Update the pointer dictionary with the parent pointer
    return standoff

def getDocumentStandoff(document):
    """Returns a list with word standoff annotations of a document tree in lxml format
    
    Generates Stand-off Annotation of a HTML document parsed to XHTML (see Forcada, Esplà-Gomis and Pérez-Ortiz, EAMT 2016) similar implementation as a set of word annotations
    """
    docstandoff=[]
    positions={document.getroot():0}
    for element in document.getroot(): #Only elements, not scripts or other kind of tags without text
        if type(element) is etree._Element:
            wordstandoff = getWordStandoff(element,document,positions)
            if wordstandoff:
                docstandoff=docstandoff+wordstandoff

    gluedStandoff = []
    buffered_standoff = None
    for annotation in docstandoff:
        if buffered_standoff == None:
            buffered_standoff = annotation
        elif annotation[0] == '+':
            buffered_standoff = buffered_standoff + annotation
        else:
            gluedStandoff.append(buffered_standoff)
            buffered_standoff = annotation
    if buffered_standoff is not None:
        gluedStandoff.append(buffered_standoff)
    return gluedStandoff

spaceEndPreviousTag=True

for line in sys.stdin:
    fields=line.split('\t')
    fields = list(map(str.strip, fields)) #Strip all elements
    fields.append(str(time.time())) #Timestamp
    fields.append(hashlib.md5(base64.b64decode(fields[0])).hexdigest()) #MD5 document checksum
    document = html5lib.parse(base64.b64decode(fields[0]),treebuilder="lxml",namespaceHTMLElements=False) #We use lxml treebuilder because of getelementpath function and iteration through elements
    fields.append(";".join(getDocumentStandoff(document)))
    print('\t'.join(fields))
