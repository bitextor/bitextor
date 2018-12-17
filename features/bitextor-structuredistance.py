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
    if len(fields) >= 7:
      #To compute the edit distance at the level of characters, HTML tags must be encoded as characters and not strings:
      tags = set(fields[6].split('_'))
      if '' in tags:
        tags.remove('')
      #List of names of tags in the current raspa
      tags_with_bounds = ["_{0}_".format(x) for x in tags]
      dic={}
      #Creating a map with all the tags found in the raspa and the character with which they will be replaced
      charidx=32
      for tag in tags_with_bounds:
        dic[chr(charidx)]=tag
        charidx=charidx+1
      #If character '_' is used, it must be replaced, since it is already representing text
      if '_' in dic:
        extratag=dic['_']
        del dic['_']
        dic[chr(charidx)]=extratag
      inv_dic = {v:k for k, v in list(dic.items())}
      for tag,code in list(inv_dic.items()):
        fields[6]=fields[6].replace(tag,code)
      docs[fileid] = fields[6]
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
    len_s = len(documents[int(fields[0])])
    sys.stdout.write(str(fields[0]))
    for j in range(1,len(fields)):
      candidate = fields[j]
      candidateid = int(fields[j].split(":")[0])
      len_t = len(documents[candidateid])
      dist = Levenshtein.distance(documents[int(fields[0])],documents[candidateid])
      port = 1 - (dist / float(max(len_s, len_t)))
      candidate+=":"+str(port)
      sys.stdout.write("\t"+candidate)
    sys.stdout.write("\n")
