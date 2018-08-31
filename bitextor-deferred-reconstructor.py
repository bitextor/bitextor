#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import time
import hashlib
import base64
import html5lib
from lxml import etree

def get_sentence(standoff,document):
    """Returns the text string from the XHTML/XML document that is pointed by the standoff annotation

    Retrieves the original content from a XHTML/XML html5lib parsed document pointed by a standoff annotation
    """
    reconstructedsentence = []
    for wordstandoff in standoff.split(';'):    #Split by standoff separator ';'
        wordstandoffsegments = wordstandoff.split('+')  #Split by '+' in case of glued annotations
        for wordstandoffseg in wordstandoffsegments:
            wordstandoffparts = wordstandoffseg.split(':')  #Obtain path and length range/limits
            wordstandofflimits = wordstandoffparts[1].split('-')
            element = document.find(wordstandoffparts[0])   #Get lxml element from path
            if element.text is None:
                element.text = ""
            if int(wordstandofflimits[1]) < len(element.text):  #Search word in element.text or in children tail depending on the limits
                if wordstandoffseg == wordstandoffsegments[0]:  #Create new annotation or add to previous word reconstruction if glued annotation
                    reconstructedsentence.append(element.text[int(wordstandofflimits[0]):int(wordstandofflimits[1])+1])
                else:
                    reconstructedsentence[-1] = reconstructedsentence[-1] + element.text[int(wordstandofflimits[0]):int(wordstandofflimits[1])+1]
            else:
                tail = element.text
                for child in element:
                    if child.tail is not None:
                        tail = tail + child.tail
                    if len(tail) > int(wordstandofflimits[1]):
                        if wordstandoffseg == wordstandoffsegments[0]:
                            reconstructedsentence.append(tail[int(wordstandofflimits[0]):int(wordstandofflimits[1])+1])
                        else:
                            reconstructedsentence[-1] = reconstructedsentence[-1] + tail[int(wordstandofflimits[0]):int(wordstandofflimits[1])+1]
                        break
    return " ".join(reconstructedsentence)



#Argument input: path of original Bitextor formatted crawl file
document_standoff = dict()
with open(sys.argv[1],'r') as reader:
    for line in reader:
        fields=line.split('\t')
        fields = list(map(str.strip, fields)) #Strip all elements
        #We use lxml treebuilder because of getelementpath function and iteration through elements
        document_standoff[fields[1]] = html5lib.parse(base64.b64decode(fields[0]),treebuilder="lxml",namespaceHTMLElements=False) #Store url:html5lib_tree for easy path search

#Input: Bitextor DOCALG file (deferred):
#url1 url2 deferred_clean_text1_in_base64 deferred_clean_text2_in_base64

#Output: Bitextor DOCALG file reconstructed:
#url1 url2 clean_text1_in_base64 clean_text2_in_base64
for line in sys.stdin:
    fields = line.split('\t')
    newfields = [fields[0],fields[1]]
    for annotation,url in {fields[2]:fields[0],fields[3]:fields[1]}.items(): #SL and TL annotations with URLs from input DOCALG file format: https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor#docalg
        newfields.append(get_sentence(annotation,document_standoff[url]))
    print("\t".join(newfields))


