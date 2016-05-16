#!__ENV__ __PYTHON__

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

import sys
from sets import Set
import argparse
from operator import itemgetter
import Levenshtein
import re
import base64

def readLETT(f, docs):
  file = open(f, "r")
  fileid = 1
  for i in file:
    fields = i.strip().split("\t")
    #Checking if format is crrect
    if len(fields) >= 5:
      rx = re.match('(https?://[^/:]+)', fields[3])
      if rx != None:
        url_domain = rx.group(1)
        url = fields[3].replace(url_domain,"")
      else:
        url = fields[3]
      docs[fileid] = url
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
    url_doc=documents[int(fields[0])]
    for j in range(1,len(fields)):
      candidate = fields[j]
      candidateid = int(fields[j].split(":")[0])
      url_candidate=documents[candidateid]
      if len(url_candidate) == 0 or len(url_doc) == 0:
        normdist = 0.0
      else:
        dist = Levenshtein.distance(url_doc,url_candidate)
        normdist=dist/float(max(len(url_doc),len(url_candidate)))
      candidate+=":"+str(normdist)
      sys.stdout.write("\t"+candidate)
    sys.stdout.write("\n")
