#!/usr/bin/env python3

#
# 1. Leer .lettr y cargarlo en memoria
# 2. Leer .ridx e ir haciendo distancia de edicion
# 3. Con el porcentaje de parecido anterior y el nuevo se realiza:
#      nuevo_porcentaje = ant_porcentaje * dist_porcentaje
#    donde:
#      dist_porcentaje = longitud(raspa1) / (longitud(raspa1) + dist)
# 4. Se muestran los 10 documentos con los porcentajes actualizados
#
# Formato final del documento:
# num_doc_lang1    [num_doc_lang2:ratio]+
#
# Genera .ridx -> reverse index
#

import os
import sys
import argparse
from operator import itemgetter
import Levenshtein
import re
import base64

pathname = os.path.dirname(sys.argv[0])
sys.path.append(pathname + "/../document-aligner")
from utils.common import open_xz_or_gzip_or_plain
#print("pathname", pathname)

def readLETT(f, docs):
  file = open_xz_or_gzip_or_plain(f)
  fileid = 1
  for i in file:
    fields = i.strip().split("\t")
    #Checking if format is crrect
    if len(fields) >= 5:
      #To compute the edit distance at the level of characters, HTML tags must be encoded as characters and not strings:
      links = re.findall('''href\s*=\s*['"]\s*([^'"]+)['"]''', base64.b64decode(fields[4]).decode("utf-8"), re.S)
      rx = re.match('(https?://[^/:]+)', fields[3])
      if rx != None:
        url_domain = rx.group(1)
        urls = "".join(links).replace(url_domain,"")
      else:
        urls = "".join(links)
      docs[fileid] = urls
    fileid += 1
  file.close()

oparser = argparse.ArgumentParser(description="Script that rescores the aligned-document candidates provided by script bitextor-idx2ridx by using the Levenshtein edit distance of the structure of the files.")
oparser.add_argument('ridx', metavar='RIDX', nargs='?', help='File with extension .ridx (reverse index) from bitextor-idx2ridx (if not provided, the script will read from the standard input)', default=None)
oparser.add_argument("-l", "--lettr", help=".lettr (language encoded and typed text with \"raspa\") file with all the information about the processed files (.lett file is also valid)", dest="lettr", required=True)
options = oparser.parse_args()

if options.ridx == None:
  reader = sys.stdin
else:
  reader = open(options.ridx,"r")

index = {}
documents = {}
readLETT(options.lettr, documents)

for i in reader:
  fields = i.strip().split("\t")
  #The document must have at least one candidate
  if len(fields)>1:
    sys.stdout.write(str(fields[0]))
    urls_doc=documents[int(fields[0])]
    for j in range(1,len(fields)):
      candidate = fields[j]
      candidateid = int(fields[j].split(":")[0])
      urls_candidate=documents[candidateid]
      if len(urls_candidate) == 0 or len(urls_doc) == 0:
        normdist = 0.0
      else:
        dist = Levenshtein.distance(urls_doc,urls_candidate)
        normdist=dist/float(max(len(urls_doc),len(urls_candidate)))
      candidate+=":"+str(normdist)
      sys.stdout.write("\t"+candidate)
    sys.stdout.write("\n")
