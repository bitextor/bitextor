import sys
import time
import hashlib
import base64
import html5lib
from lxml import etree

def getWordStandoff(element,document,positionsdict):
    """Returns a list with word standoff annotations of a tree element of lxml
    
    It is
    """
    standoff=[]                
    wordposition=0      #Pointer of actual start word character inside the element tag
    if element.text != None:        #If we have text in tag
        if element.text[0] == ' ':      #If the text inside the tag begins with a space character, increase the pointer
            wordposition = wordposition+1
        for word in element.text.split():
            #For every space separated word inside the tag, create a standoff annotation with the path from root, remove the w3 note from tag and the initial and final character positions. Then, update the position pointer
            standoff.append(document.getelementpath(element).replace('{http://www.w3.org/1999/xhtml}','')+':'+str(wordposition)+"-"+str(wordposition+len(word)-1))
            wordposition=wordposition+len(word)
        if element.text[-1] == ' ':     #If the text inside the tag ends with a space character, increase the pointer
            wordposition = wordposition+1
    if element.tail != None:        #If we have tail text (text after the closing tag until the next open/close tag)
        wordpositionparent=positionsdict[element.getparent()]       #Pointer of actual start word character outside the element tag (tail), which belongs to the parent tag
        if element.tail[0] == ' ':      #If the tail text begins with a space character, increase the pointer
            wordpositionparent = wordpositionparent+1
        for word in element.tail.split():
            #Generate word standoff annotation in the same way as the text inside the tag
            standoff.append(document.getelementpath(element.getparent()).replace('{http://www.w3.org/1999/xhtml}','')+':'+str(wordpositionparent)+"-"+str(wordpositionparent+len(word)-1))
            wordpositionparent=wordpositionparent+len(word)
        if element.tail[-1] == ' ':     #If the tail text ends with a space character, increase the pointer
            wordpositionparent = wordpositionparent+1
        positionsdict[element.getparent()]=wordpositionparent       #Update the pointer dictionary with the parent pointer
    positionsdict[element]=wordposition     #Insert the element pointer in the pointer dictionary (for future children with tail text)
    return standoff

def getDocumentStandoff(document):
    """Returns a list with word standoff annotations of a document tree in lxml format
    
    Generates Stand-off Annotation of a HTML document parsed to XHTML (see Forcada, Esplà-Gomis and Pérez-Ortiz, EAMT 2016) similar implementation as a set of word annotations
    """
    docstandoff=[]
    positions=dict()
    for element in document.iter(tag=etree.Element):
        wordstandoff = getWordStandoff(element,document,positions)
        if wordstandoff:
            docstandoff=docstandoff+wordstandoff
    return docstandoff

for line in sys.stdin:
    fields=line.split('\t')
    fields = list(map(str.strip, fields)) #Strip all elements
    fields.append(str(time.time())) #Timestamp
    fields.append(hashlib.md5(base64.b64decode(fields[0])).hexdigest()) #MD5 document checksum
    document = html5lib.parse(base64.b64decode(fields[0]),treebuilder="lxml")
    fields.append(";".join(getDocumentStandoff(document)))
    print('\t'.join(fields))
