#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from wagnerfischer import WagnerFischer
import base64
import html5lib
from lxml import etree
import re


#Argument input: deferred Bitextor crawl file with plain text and its standoff annotations (output of deferred document script)
documentStandoff = dict()
with open(sys.argv[1],'r') as reader:
    for line in reader:
        fields = line.split('\t')
        fields = list(map(str.strip, fields))
        documenttext = base64.b64decode(fields[0]).decode('utf8')
        documentStandoff[fields[1]]=(documenttext,fields[4].split(';'))

#Input: Bitextor DOCALG file:
#url1 url2 clean_text1_in_base64 clean_text2_in_base64
for line in sys.stdin:
    fields = line.split('\t')
    fields = list(map(str.strip, fields)) #Strip all elements

    shortpathSL=WagnerFischer(fields[2].split(' '),[x for x in documentStandoff[fields[0]][0].replace('\n',' ').replace('\t',' ').replace('\xa0',' ').split(' ') if x != '']).optimum_alignments() #Calculate a short distance path using Wagner-Fischer algorithm for source
    shortpathTL=WagnerFischer(fields[3].split(' '),[x for x in documentStandoff[fields[1]][0].replace('\n',' ').replace('\t',' ').replace('\xa0',' ').split(' ') if x != '']).optimum_alignments() #and target sentences
    
    position=0
    standoffSL=[]
    if shortpathSL[0] == 'S' and len(shortpathSL) == shortpathSL.count('I')+1:
        fields.append("")
    else:
        for op in shortpathSL: #Obtain the standoff annotation of each sentence word from the full annotated document they come from, counting non-inserted words
            if op != "I":
                standoffSL.append(documentStandoff[fields[0]][1][position])
            position = position + 1
        fields.append(";".join(standoffSL))

    position=0
    standoffTL=[]
    if shortpathTL[0] == 'S' and len(shortpathTL) == shortpathTL.count('I')+1:
        standoffTL=[]
        fields.append("")
    else:
        for op in shortpathTL: #Same for TL
            if op != "I":
                standoffTL.append(documentStandoff[fields[1]][1][position])
            position = position + 1
        fields.append(";".join(standoffTL))

    #TODO: simplify the sentence standoff annotation joining/collapsing word standoff annotations with the same tag path

    print("\t".join(fields))
