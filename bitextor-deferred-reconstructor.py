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
    reconstructedsentence = ""
    for wordstandoff in standoff.split(';'):
        wordstandoffparts = wordstandoff.split(':')
        wordstandofflimits = wordstandoffparts[1].split('-')
        element = document.find(wordstandoffparts[0])
        if int(wordstandofflimits[1]) < len(element.text):
            reconstructedsentence = reconstructedsentence + element.text[int(wordstandofflimits[0])-int(wordstandofflimits[1])]
        else:
            tail = element.text
            for child in element:
                tail = tail + child.tail
                if len(tail) > int(wordstandofflimits[1]):
                    reconstructedsentence = reconstructedsentence + tail[int(wordstandofflimits[0])-int(wordstandofflimits[1])]
                    break
    return reconstructedsentence




document_standoff = dict()
with open(sys.argv[1],'r') as reader:
    for line in reader:
        fields=line.split('\t')
        fields = list(map(str.strip, fields)) #Strip all elements
        fields.append(str(time.time())) #Timestamp
        fields.append(hashlib.md5(base64.b64decode(fields[0])).hexdigest()) #MD5 document checksum
        document_standoff[fields[1]] = html5lib.parse(base64.b64decode(fields[0]),treebuilder="lxml") #We use lxml treebuilder because of getelementpath function and iteration through elements
    
for line in sys.stdin:
    fields = line.split('\t')
    newfields = [fields[0],fields[1]]
    for annotation,url in {fields[2]:fields[0],fields[3]:fields[1]}.items():
        newfields.append(get_sentence(annotation,document_standoff[url]))
    print("\t".join(newfields))


