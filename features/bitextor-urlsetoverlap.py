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
import re
import base64

pathname = os.path.dirname(sys.argv[0])
sys.path.append(pathname + "/../document-aligner")
from utils.common import open_xz_or_gzip_or_plain
#print("pathname", pathname)

def extract_urls(f, docs):
  with open_xz_or_gzip_or_plain(f) as fd:
    fileid = 1
    for i in fd:
      #To compute the edit distance at the level of characters, HTML tags must be encoded as characters and not strings:
      links = re.findall('''href\s*=\s*['"]\s*([^'"]+)['"]''', base64.b64decode(i.strip()).decode("utf-8"), re.S)
      docs[fileid] = set(list(links))
      fileid += 1

oparser = argparse.ArgumentParser(description="Script that rescores the aligned-document candidates provided by script bitextor-idx2ridx by using the Levenshtein edit distance of the structure of the files.")
oparser.add_argument('ridx', metavar='RIDX', nargs='?', help='File with extension .ridx (reverse index) from bitextor-idx2ridx (if not provided, the script will read from the standard input)', default=None)
oparser.add_argument("--html", help="File produced during pre-processing containing all HTML files in a WARC file", dest="html", required=True)
options = oparser.parse_args()

if options.ridx == None:
  reader = sys.stdin
else:
  reader = open(options.ridx,"r")

index = {}
documents = {}
extract_urls(options.html, documents)

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
      if len(urls_doc.union(urls_candidate)) > 0:
        bagofurlsoverlap=len(urls_doc.intersection(urls_candidate))/float(len(urls_doc.union(urls_candidate)))
      else:
        bagofurlsoverlap=0
      candidate+=":"+str(bagofurlsoverlap)
      sys.stdout.write("\t"+candidate)
    sys.stdout.write("\n")
