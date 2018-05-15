#!__ENV__ __PYTHON__

import sys
import argparse

oparser = argparse.ArgumentParser(description="Script that reads the output of bitextor pipeline and add stats from ILSP-FC")
oparser.add_argument('aligned_seg', metavar='FILE', nargs='?', help='File containing the set of aliged segments (if undefined, the script reads from the standard input)', default=None)
oparser.add_argument("-s", "--stats", help="Print stats or just output the input", action="store_true", dest="isPrintingStats", default=False)
oparser.add_argument("-f", "--filtering", help="Filter lines according to ELRC rules (printing stats required)", action="store_true", dest="isFiltering", default=False)
oparser.add_argument("-c", "--columns", help="Name of columns of the input tab separated file split by comma. Default: url1,url2,seg1,seg2,hunalign,zipporah,bicleaner", default="url1,url2,seg1,seg2,hunalign,zipporah,bicleaner")

options = oparser.parse_args()

if options.aligned_seg != None:
  reader = open(options.aligned_seg,"r")
else:
  reader = sys.stdin

idcounter=0
columns = options.columns.split(',')

for i in reader:
  idcounter = idcounter+1
  fields = i.split("\t")
  fields[-1]=fields[-1].strip()
  fieldsdict = dict()
  extracolumns=["idnumber"]

  for field,column in zip(fields,columns):
    fieldsdict[column]=field
  if options.isPrintingStats:
    extracolumns=["lengthratio","numTokensSL","numTokensTL","idnumber"]
    if len(fieldsdict["seg2"].decode('utf8')) == 0:
      lengthRatio=0
    else:
      lengthRatio=len(fieldsdict["seg1"].decode('utf8'))*1.0/len(fieldsdict["seg2"].decode('utf8'))
    numTokensSL=len(fieldsdict["seg1"].split(' ')) #This is not the way this should be counted, we need to tokenize better first
    numTokensTL=len(fieldsdict["seg2"].split(' ')) #This is not the way this should be counted, we need to tokenize better first
    fieldsdict["lengthratio"]=str(lengthRatio)
    fieldsdict["numTokensSL"]=str(numTokensSL)
    fieldsdict["numTokensTL"]=str(numTokensTL)
    if options.isFiltering:
      if "zipporah" in fieldsdict:
        fieldsdict["zipporah"]=str(round(float(fieldsdict["zipporah"]),4))
      if "bicleaner" in fieldsdict and fieldsdict["bicleaner"].strip() != '':
        fieldsdict["bicleaner"]=str(round(float(fieldsdict["bicleaner"]),4))
      if int(fieldsdict["numTokensSL"]) >= 200 or int(fieldsdict["numTokensTL"]) >= 200 or fieldsdict["seg1"].strip() == '' or fieldsdict["seg2"].strip() == '' or float(fieldsdict["lengthratio"]) >= 6 or float(fieldsdict["lengthratio"]) <= 0.1666: 
        continue
  fieldsdict["idnumber"]=str(idcounter)
  fieldstoprint=[]
  for column in columns+extracolumns:
    fieldstoprint.append(fieldsdict[column])
  print("\t".join(fieldstoprint))


