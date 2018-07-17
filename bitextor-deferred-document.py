import sys
import time
import hashlib
import base64
import html5lib

def getWordStandoff(element,document):
    standoff=[]
    if element.text != None:
        wordposition=0
        alltext = element.text
        if element.tail != None:
            alltext = alltext+element.tail
        for word in alltext.split():
            standoff.append(document.getelementpath(element).replace('{http://www.w3.org/1999/xhtml}','')+':'+str(wordposition)+"-"+str(wordposition+len(word)-1))
            wordposition=wordposition+len(word)
    return standoff
def getDocumentStandoff(document):
    docstandoff=[]
    for element in document.iter():
        wordstandoff = getWordStandoff(element,document)
        if wordstandoff:
            docstandoff=docstandoff+wordstandoff
    return docstandoff

for line in sys.stdin:
    fields=line.split('\t')
    fields = list(map(str.strip, fields)) #Strip all elements
    fields.append(str(time.time()))
    fields.append(hashlib.md5(base64.b64decode(fields[0])).hexdigest())
    document = html5lib.parse(base64.b64decode(fields[0]),treebuilder="lxml")
    fields.append(";".join(getDocumentStandoff(document)))
    print('\t'.join(fields))
